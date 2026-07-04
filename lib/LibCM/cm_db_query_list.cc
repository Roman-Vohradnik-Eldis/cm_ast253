#include "cm_db_query_list.hh"
#include "cm_db_query_state.hh"
#include "cm_db_types.hh"
#include "cm_write_binary.hh"
#include "compress_base.hh"
#include "weak_string.hh"

#include <algorithm>
#include <stdint.h>
#include <inttypes.h>

#define DB_DEBUG(level, fmt, ...) cms_ns2_if_print("cmdb_query", debug_name, level, fmt, ##__VA_ARGS__)

MyQueryState *find_by_user_query_id(std::vector<MyQueryState> &queries, QueryId id)
{
    for (MyQueryState &q : queries)
    {
        if (q.user_query_id == id)
            return &q;
    }
    return nullptr;
}

MyQueryList::MyQueryList(std::string dbg_name)
    : debug_name(dbg_name)
{
}

std::string query_to_string(CmDbQuery const &query)
{
    std::vector<Expr> flags;
    
    std::vector<Expr> commands;
    for (CmDbCommand const &c : query.commands)
        commands.push_back(cm_make_list(c.cmd, c.args));
    
    return to_string(Expr(cm_make_list(flags, std::move(commands))));
}

/**
 * Parse (maybe partial) response.
 *
 * @param eof: true when the response is completed and this is the
 * last chunk of the message.
 * 
 * @return true on success, false on error.
 */
bool process_received_response(MyQueryState &q, WeakString debug_name, bool eof)
{
    DB_DEBUG(4, "process_received_response: %zu bytes: >>%s<<", q.response_buffer.size(), q.response_buffer.str().c_str());
    try
    {
        char const *data = (char const *)q.response_buffer.data();
        q.expr_parser.add_string(WeakString(data, data + q.response_buffer.size()));
        if (eof)
            q.expr_parser.add_eof();

        // Pokud jsme se dostali sem, tak to parser uspesne schroupal,
        // bez vyhozeni vyjimky.
        q.response_buffer.clear();
        
        std::vector<Expr> parsed_exprs = q.expr_parser.get_result();
        for (Expr const &e : parsed_exprs)
        {
            if (e.size() != 4)
                throw std::runtime_error("bad list length");
            int64_t cmd_id = e.at(0).to_int();
            if (cmd_id < 0)
                throw std::runtime_error("bad cmd_id");
            // int64_t status = e->at(1).to_int();
            // if (status < 0 || status > 2)
            //     throw std::runtime_error("bad status");
            CmDbCommandStatus status = read_command_status(e.at(1).to_string());
            std::vector<Expr> rows;
            for (Expr const &x : e.at(2))
                rows.push_back(x);
            std::string const &err_msg = e.at(3).to_string();
            
            add_to_query_state(q.received_responses,
                               (size_t)cmd_id,
                               status,
                               std::move(rows),
                               err_msg);
        }
        return true;
    }
    catch (std::runtime_error &e)
    {
        DB_DEBUG(1, "Failed to parse response >>%s<<: %s", q.response_buffer.str().c_str(), e.what());
        return false;
    }
}

bool MyQueryList::has_query_id(QueryId query_id)
{
    for (MyQueryState &q : queries)
    {
        if (q.user_query_id == query_id)
            return true;
    }
    return false;
}

void MyQueryList::generate_data_to_send(OutputBufferSimple &out)
{
    size_t max_size = out.get_max_size();
    if (max_size <= query_header_length())
        return;
    max_size -= query_header_length();
    
    for (MyQueryState &q : queries)
    {
        if (q.query_status == QueryStatus::NeedSendCancel)
        {
            // TODO: Pokud by se tam chybova zprava nevesla, tak se
            // orizne. Chtelo by to pockat na vice mista, nebo zavest
            // 'e', 'E'.
            // size_t len = std::min(q.cancel_message.size(), max_size);
            // DB_DEBUG(4, "cancel_size=%zu, max=%zu", q.cancel_message.size(), len);
            std::vector<uint8_t> msg;
            // msg.push_back('C');
            push_hex_number(msg, q.service_id.value, 4);
            push_hex_number(msg, q.user_query_id.value, 16);
            // assert(msg.size() == query_header_length());

            ssize_t len = out.try_add_message('C', 'C', msg, view_bytes(q.cancel_message), 0);
            
            // msg.insert(msg.end(), q.cancel_message.begin(), q.cancel_message.begin() + len);

            if (len >= 0)
            {
                q.query_status = QueryStatus::CancelWasSent;
                return;
            }
        }
        if (q.query_status == QueryStatus::Running || q.query_status == QueryStatus::Prepared)
        {
            if (!q.query_buffer.empty())
            {
                // size_t size = std::min(max_size, q.query_buffer.size());
                // bool is_complete = size >= q.query_buffer.size() && q.query_status == QueryStatus::Prepared;

                std::vector<uint8_t> msg;
                // msg.push_back(is_complete ? 'Q' : 'q');
                push_hex_number(msg, q.service_id.value, 4);
                push_hex_number(msg, q.user_query_id.value, 16);
                // assert(msg.size() == query_header_length());
                // msg.insert(msg.end(), q.query_buffer.begin(), q.query_buffer.begin() + size);

                char completed_char = q.query_status == QueryStatus::Prepared ? 'Q' : 'q';
                ssize_t len = out.try_add_message('q', completed_char,
                                                  msg, view_bytes(q.query_buffer.begin(), q.query_buffer.end()),
                                                  1);
                if (len >= 0)
                {
                    assert(len > 0);
                    q.query_buffer.advance(len);
                    return;
                }
            }
        }
    }
}

void set_each_command_response_to_error(std::vector<MyReceivedResponse> &responses, std::string const &msg)
{
    for (MyReceivedResponse &r : responses)
    {
        if (r.cmd_status == CM_DB_ROWS)
        {
            r.cmd_status = CM_DB_ERROR;
            r.was_read = false;
            r.error_message = msg;
        }
    }
}

/**
 * Presuneme string z `response_buffer` do parseru. Vubec nevadi,
 * pokud je v `response_buffer` jenom kousek Expr, parser si stav
 * zapamatuje.
 *
 * On error, it changes the query state.
 *
 * @return true if there are new data to the callback
 */
bool parse_received_data(MyQueryState &q, std::string const &debug_name)
{
    assert(!q.is_forwarded_query);
    assert(q.response_status == ResponseStatus::Running || q.response_status == ResponseStatus::Completed);
    if (!process_received_response(q, debug_name, q.response_status == ResponseStatus::Completed))
    {
        // Failed to parse 
        q.query_status = QueryStatus::NeedSendCancel;
        q.response_status = ResponseStatus::Aborted;
        q.cancel_message = "Failed to parse";
        if (!q.is_forwarded_query)
        {
            set_each_command_response_to_error(q.received_responses, q.cancel_message);
        }
    }
            
    if (q.response_status == ResponseStatus::Completed)
    {
        size_t remaining = q.response_buffer.size();
        if (remaining > 0)
        {
            DB_DEBUG(1, "Unused characters in the response buffer: %zu characters", remaining);
        }
        set_each_command_response_to_error(q.received_responses, "Response not received");
    }
    return true;
}

bool MyQueryList::receive_data(PartnerQueryId query_id, ServiceId service_id, View<uint8_t> data, char kind)
{
    assert(kind == 'r' || kind == 'R' || kind == 'E');
    DB_DEBUG(4, "receive_data: PartnerQueryId=%llx kind=%c, data=%s, kind=%c",
             query_id.value, kind, std::string(data.begin(), data.end()).c_str(), kind);

    MyQueryState *qp = find_by_user_query_id(queries, QueryId{query_id.value});
    if (!qp)
    {
        DB_DEBUG(1, "receive_data: Query id was not found");
        return false;
    }
    MyQueryState &q = *qp;
    if (q.service_id != service_id)
    {
        DB_DEBUG(1, "Invalid service_id");
        return false;
    }

    if (q.response_status == ResponseStatus::Cancelled)
    {
        DB_DEBUG(3, "Response discarded because the query was cancelled");
        return false;
    }
    
    if (q.response_status == ResponseStatus::Completed)
    {
        DB_DEBUG(1, "Response already completed, ignoring message");
        return false;
    }
    if (q.response_status == ResponseStatus::Aborted)
    {
        DB_DEBUG(1, "Response already aborted, ignoring message");
        return false;
    }

    assert(q.response_status == ResponseStatus::Running);
    if (kind == 'r' || kind == 'R')
    {
        if (kind == 'R')
        {
            // DB_DEBUG(1, "Response completed")
            q.response_status = ResponseStatus::Completed;
        }

        // Tohle je kvuli kompatibilite se starsi verzi protokolu
        if (q.bytes_received == 0 && data.size() > 0)
        {
            assert(data[0] == 't');
            if (data[0] == 't')
                data.advance(1);
        }

        q.bytes_received += data.size();
        q.response_buffer.append(data);
        if (q.is_forwarded_query)
        {
            // Nic nedelame, cekame, az si ji nekdo vyzvedne.
            return true;
        }
        else
        {
            return parse_received_data(q, debug_name);
        }
    }
    else
    {
        assert(kind == 'E');
        q.query_status = QueryStatus::Aborted;
        q.response_status = ResponseStatus::Aborted;
        std::string error_message(data.begin(), data.end());
        if (!q.is_forwarded_query)
            set_each_command_response_to_error(q.received_responses, error_message);
        return true;
    }
}

bool query_id_exists(std::vector<MyQueryState> const &queries, QueryId id)
{
    for (MyQueryState const &q : queries)
    {
        if (q.user_query_id == id)
        {
            return true;
        }
    }
    return false;
}

void MyQueryList::begin_forwarded_query(QueryId user_id, ServiceId service_id, DbService service_name)
{
    if (query_id_exists(queries, user_id))
    {
        DB_DEBUG(1, "MyQueryList::add_new_query: Logic error: UserQueryId already exists");
        return;
    }
    queries.push_back(make_forwarded_query(user_id, service_id, service_name));
}

void MyQueryList::add_forwarded_query_data(QueryId user_id, bool was_completed, View<uint8_t> data)
{
    // DB_DEBUG(3, "add_forwarded_query_data: id=%" PRIx64 ", was_completed=%d, size=%zu", user_id.value, was_completed, data.size());
    MyQueryState *qp = find_by_user_query_id(queries, user_id);
    if (!qp)
    {
        DB_DEBUG(1, "Cannot append data. Unknown queey id");
        return;
    }
    MyQueryState &q = *qp;
    if (!q.is_forwarded_query)
    {
        DB_DEBUG(1, "Cannot append data. Query is not forwarded query");
        return;
    }
    if (q.query_status == QueryStatus::Prepared)
    {
        DB_DEBUG(1, "Cannot append data. Query was already prepared");
        return;
    }
    if (q.query_status != QueryStatus::Running)
    {
        DB_DEBUG(1, "Cannot append data. Query is not running");
        return;
    }
    q.query_buffer.append(data);
    if (was_completed)
    {
        q.query_status = QueryStatus::Prepared;
    }
}

void MyQueryList::add_new_query(QueryId user_id, ServiceId service_id, DbService service_name,
                                CmDbQuery const &query,
                                int64_t time_now) 
{
    if (query_id_exists(queries, user_id))
    {
        DB_DEBUG(1, "MyQueryList::add_new_query: Logic error: UserQueryId already exists");
        return;
    }
    int64_t timeout = query.timeout_usec.get_or(60*1000*1000);
    queries.push_back(make_new_query(user_id,
                                     service_id,
                                     service_name,
                                     query_to_string(query),
                                     query.commands.size(),
                                     timeout,
                                     time_now));
    cms_ns_if_print("profile", 3, "add_new_query, active_query_count=%zu", queries.size());
}

void MyQueryList::remove_finished_queries()
{
    auto it = std::remove_if(queries.begin(), queries.end(), can_query_be_erased);
    for (auto i = it; i != queries.end(); ++i)
        DB_DEBUG(4, "Query completed: %llx", i->user_query_id.value);
    queries.erase(it, queries.end());
}

bool is_query_completed(std::vector<MyReceivedResponse> const &rrs)
{
    return std::all_of(rrs.begin(), rrs.end(), [](MyReceivedResponse const &rr){
        return rr.was_read && rr.cmd_status != CM_DB_ROWS;
    });
}
    
std::vector<CmDbReceivedResponse> MyQueryList::get_new_responses(size_t limit)
{
    std::vector<CmDbReceivedResponse> result;
    for (MyQueryState &q : queries)
    {
        if (q.is_forwarded_query)
            continue;

        if (q.response_status == ResponseStatus::Cancelled)
            continue;

        for (size_t i = 0; i < q.received_responses.size(); ++i)
        {
            MyReceivedResponse &rr = q.received_responses[i];
            if (!rr.was_read && (rr.cmd_status != CM_DB_ROWS || !rr.rows.empty()))
            {
                result.push_back(CmDbReceivedResponse({
                            q.user_query_id,
                            i,
                            q.service_name,
                            rr.cmd_status,
                            std::move(rr.rows),
                            std::move(rr.error_message),
                            false}));
                rr.was_read = true;
                if (is_query_completed(q.received_responses))
                    result.back().is_query_completed = true;
                if (limit > 0 && result.size() >= limit)
                    break;
            }
        }
    }
    remove_finished_queries();
    return result;
}

// Od partnera dostaneme odpoved na nas dotaz, ted ji predame uzvateli
std::vector<DbMessage> MyQueryList::get_forwarded_responses()
{
    std::vector<DbMessage> result;
    for (MyQueryState &q : queries)
    {
        if (!q.is_forwarded_query)
            continue;

        if (q.response_status == ResponseStatus::Aborted)
        {
            DbMessage msg{'E', q.service_name, q.user_query_id, {}};
            msg.data.insert(msg.data.end(), q.abort_message.begin(), q.abort_message.end());
            DB_DEBUG(4, "get_forwarded_responses: Aborted: %s", to_string(msg).c_str());
            result.push_back(std::move(msg));
            q.response_status = ResponseStatus::Cancelled; // Uzivateli uz nic nepujde
        }
        else if (q.response_status == ResponseStatus::Running || q.response_status == ResponseStatus::Completed)
        {
            char kind = q.response_status == ResponseStatus::Completed ? 'R' : 'r'; 
            DbMessage msg{kind, q.service_name, q.user_query_id, {}};
            msg.data.insert(msg.data.end(), q.response_buffer.begin(), q.response_buffer.end());
            DB_DEBUG(4, "get_forwarded_responses: Good: %s", to_string(msg).c_str());
            q.response_buffer.clear();
            result.push_back(std::move(msg));
        }
        else if (q.response_status == ResponseStatus::Cancelled)
        {
            // Uzivatel odpoved nechce
        }
        else
        {
            assert(false);
        }
    }
    remove_finished_queries();
    return result;
}

void MyQueryList::disconnect()
{
    for (MyQueryState &q : queries)
    {
        q.query_status = QueryStatus::Aborted;
        if (q.response_status == ResponseStatus::Running)
        {
            q.response_status = ResponseStatus::Aborted;
            q.abort_message = "Disconnected";
            if (!q.is_forwarded_query)
                set_each_command_response_to_error(q.received_responses, "Disconnected");
        }
    }
}

size_t MyQueryList::size() const
{
    return queries.size();
}

void MyQueryList::cancel_query(QueryId query_id, WeakString reason, bool generate_callback)
{
    DB_DEBUG(3, "cancel_query %llx (cb=%d): %s", (long long)query_id.value, generate_callback, reason.str().c_str());
    MyQueryState *qp = find_by_user_query_id(queries, query_id);
    if (!qp)
    {
        DB_DEBUG(1, "cancel_query: Query %llx was not found", (long long)query_id.value);
        return;
    }
    MyQueryState &q = *qp;
    if (q.query_status == QueryStatus::NeedSendCancel || q.query_status == QueryStatus::CancelWasSent)
    {
        DB_DEBUG(2, "cancel_query: Query %llx was already cancelled", (long long)query_id.value);
        return;
    }

    if (q.query_status == QueryStatus::Aborted)
    {
        DB_DEBUG(2, "cancel_query: Query %llx was already aborted", (long long)query_id.value);
        return;
    }

    assert(q.query_status == QueryStatus::Prepared || q.query_status == QueryStatus::Running);

    // Chceme pri nejblizsi prilezitosti poslat cancel
    q.query_status = QueryStatus::NeedSendCancel;
    q.cancel_message = reason;

    // Jak nalozime s odpovedi
    if (!generate_callback)
    {
        q.response_status = ResponseStatus::Cancelled;
    }
    else
    {
        q.response_status = ResponseStatus::Aborted;
        if (!q.is_forwarded_query)
        {
            set_each_command_response_to_error(q.received_responses, q.cancel_message);
        }
    }

    // If a query is completed and we cancel it, it is finished.
    remove_finished_queries();
}

bool MyQueryList::check_timeout(int64_t time_now)
{
    bool changed = false;
    for (MyQueryState &q : queries)
    {
        DB_DEBUG(4, "Check timeout: timeout after %lld usec (timeout was set to %lld usec)",
                 (long long)(q.time_timeout - time_now),
                 (long long)(q.time_timeout - q.time_created));
        if (q.time_timeout < time_now)
        {
            if (q.response_status == ResponseStatus::Running)
            {
                q.query_status = QueryStatus::NeedSendCancel;
                q.response_status = ResponseStatus::Aborted;
                q.abort_message = format("Timeout after %.3f msec", (time_now - q.time_created) / 1000.0);
                q.cancel_message = q.abort_message;

                if (!q.is_forwarded_query)
                {
                    set_each_command_response_to_error(q.received_responses, q.abort_message);
                }
                changed = true;
            }
            else
            {
                assert(q.query_status != QueryStatus::Running);
            }
        }
    }
    return changed;
}

Maybe<int64_t> MyQueryList::get_next_wakeup_time()
{
    Maybe<int64_t> min;
    for (MyQueryState &q : queries)
    {
        if (q.response_status == ResponseStatus::Running)
        {
            if (!min)
                min = q.time_timeout;
            else if (q.time_timeout < *min)
                min = q.time_timeout;
        }
    }
    return min;
}

