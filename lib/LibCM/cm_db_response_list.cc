#include "cm_db_response_list.hh"
#include "cm_db_response_status.hh"
#include "cm_db_types.hh"
#include "cm_string_buffer.hh"
#include "cm_utils.hh"
#include "cm_write_binary.hh"
#include "compress_base.hh"
#include "compress_zstd.hh"
#include "expr_parser.hh"

#define DB_DEBUG(level, fmt, ...) cms_ns2_if_print("cmdb_response", debug_name, level, fmt, ##__VA_ARGS__)

MyResponseState *find_by_user_query_id(std::list<MyResponseState> &queries, QueryId id)
{
    for (MyResponseState &q : queries)
    {
        if (q.user_query_id == id)
            return &q;
    }
    return nullptr;
}

MyResponseState *find_by_partner_query_id(std::list<MyResponseState> &queries, PartnerQueryId id)
{
    for (MyResponseState &q : queries)
    {
        if (q.partner_query_id == id)
            return &q;
    }
    return nullptr;
}

MyResponseList::MyResponseList(std::string dbg_name)
    : debug_name(dbg_name)
{}

bool MyResponseList::has_query(PartnerQueryId query_id)
{
    for (MyResponseState &q : queries)
    {
        if (q.partner_query_id == query_id)
            return true;
    }
    return false;
}

void MyResponseList::create_new_query(QueryId user_id,
                                      PartnerQueryId partner_id,
                                      ServiceId service_id,
                                      DbService service_name,
                                      bool forward,
                                      bool compress)
{
    assert(!(forward && compress));
    for (MyResponseState &q : queries)
    {
        if (q.user_query_id == user_id)
        {
            DB_DEBUG(1, "UserQueryId already exists");
            return;
        }
        if (q.partner_query_id == partner_id)
        {
            DB_DEBUG(1, "UserQueryId already exists");
            return;
        }
    }
    DB_DEBUG(4, "Created new query: user_id=%llx, partner_id=%llx, service_id=%llx, service_name=%s, forward=%d, compress=%d",
             (long long)user_id.value,
             (long long)partner_id.value,
             (long long)service_id.value,
             service_name.c_str(),
             forward,
             compress);
    queries.push_back({user_id, partner_id, service_id, service_name, forward, compress});
    cms_ns_if_print("profile", 3, "add_received_query, active_receieved_query_count=%zu", queries.size());
}

std::vector<CmDbReceivedQuery> MyResponseList::get_new_queries(size_t limit)
{
    std::vector<CmDbReceivedQuery> result;
    for (MyResponseState &q : queries)
    {
        if (q.is_forwarded_query)
            continue;
        if (q.query && !q.query_was_read)
        {
            result.push_back(CmDbReceivedQuery({q.user_query_id, q.service_name, q.query->commands}));
            q.query_was_read = true;
        }
        if (limit > 0 && result.size() >= limit)
            break;
    }
    return result;
}

std::vector<DbMessage> MyResponseList::get_queries_to_forward()
{
    std::vector<DbMessage> result;
    for (MyResponseState &q : queries)
    {
        if (q.query_was_cancelled)
            continue;
        
        if (q.is_forwarded_query && !q.query_was_read)
        {
            if (!q.query_buffer.empty())
            {
                char kind = q.query_was_completed ? 'Q' : 'q'; 
                DbMessage msg{kind, q.service_name, q.user_query_id, {}};
                std::swap(msg.data, q.query_buffer);
                DB_DEBUG(4, "get_queries_to_forward: Good: %s", to_string(msg).c_str());
                if (q.query_was_completed)
                    q.query_was_read = true;
                result.push_back(msg);
            }
        }
    }
    return result;
}

bool MyResponseList::has_query_id(QueryId query_id)
{
    for (MyResponseState &q : queries)
    {
        if (q.user_query_id == query_id)
            return true;
    }
    return false;
}

std::set<QueryId> MyResponseList::get_all_query_ids()
{
    std::set<QueryId> result;
    for (MyResponseState &q : queries)
        result.insert(q.user_query_id);
    return result;
}

void MyResponseList::send_forwarded_response(QueryId query_id, View<uint8_t> data, char kind)
{
    assert(kind == 'r' || kind == 'R' || kind == 'E');
    MyResponseState *qp = find_by_user_query_id(queries, query_id);
    if (!qp)
    {
        DB_DEBUG(1, "add_forwarded_response: Unknown query id %llx", (long long)query_id.value);
        return;
    }
    MyResponseState &q = *qp;
    
    DB_DEBUG(4, "add_forwarded_response: user_id=%llx, partner_id=%llx",
             q.user_query_id.value,
             q.partner_query_id.value);
    if (!q.is_forwarded_query)
    {
        DB_DEBUG(1, "add_forwarded_response: not a forwarded query");
        return;
    }
    if (q.query_was_cancelled)
    {
        DB_DEBUG(1, "add_forwarded_response: query was cancelled");
        return;
    }
    if (q.all_responses_prepared)
    {
        DB_DEBUG(1, "add_forwarded_response: Responses are already prepared");
        return;
    }
    if (kind == 'E')
    {
        q.responses_error = true;
        q.all_responses_prepared = true;
    }
    else
    {
        assert(kind == 'r' || kind == 'R');
        DB_DEBUG(4, "add_forwarded_response: %s", std::string(data.begin(), data.end()).c_str());
        q.response_buffer.append(data);
        if (kind == 'R')
        {
            DB_DEBUG(4, "Setting all_responses_prepared to true");
            q.all_responses_prepared = true;
        }
    }
}

// bool all_responses_finished_and_read(std::vector<MyReceivedResponse> const &responses)
// {
//     for (MyReceivedResponse const &r : responses)
//         if (r.cmd_status == CM_DB_ROWS || !r.was_read)
//             return false;
//     return true;
// }

void MyResponseList::send_response(QueryId query_id,
                                   size_t cmd_id,
                                   CmDbCommandStatus status,
                                   std::vector<Expr> const &rows,
                                   std::string const &err_msg)
{
    MyResponseState *qp = find_by_user_query_id(queries, query_id);
    if (!qp)
    {
        DB_DEBUG(1, "add_received_response: Unknown query id %llx", query_id.value);
        return;
    }
    MyResponseState &q = *qp;
    DB_DEBUG(4, "add_received_response: user_id=%llx, partner_id=%llx",
             q.user_query_id.value,
             q.partner_query_id.value);
    if (q.all_responses_prepared)
    {
        DB_DEBUG(1, "add_received_response: Response already prepared");
        return;
    }
    if (q.query_was_cancelled)
    {
        DB_DEBUG(1, "add_received_response: Query was already cancelled");
        return;
    }
    add_to_query_state(q.responses, cmd_id, status, rows, std::move(err_msg));
    q.rows_added += rows.size();
    auto pred = [](MyReceivedResponse &r){return r.cmd_status != CM_DB_ROWS;};
    q.all_responses_prepared = std::all_of(q.responses.begin(), q.responses.end(), pred);
    if (q.all_responses_prepared)
    {
        DB_DEBUG(3, "add_received_response: Response is now prepared");
    }
}

std::vector<CmDbCommand> parse_commands(Expr const &cmdlist)
{
    std::vector<CmDbCommand> commands;
    for (Expr const &p : cmdlist)
    {
        if (p.size() != 2)
            throw std::runtime_error("expected pair");
        std::string const &cmd = p.at(0).to_string();
        Expr const &args = p.at(1);
        std::vector<Expr> v;
        std::copy(args.begin(), args.end(), std::back_inserter(v));
        commands.push_back(CmDbCommand{cmd, v});
    }
    return commands;
}

Maybe<CmDbQuery> parse_query(WeakString msg, WeakString debug_name)
{
    try
    {
        DB_DEBUG(4, "parse_string: %s", msg.str().c_str());
        Expr e = parse_one(msg);

        // Parse flags
        bool use_compression = false;
        Expr const &flags = e.at(0);
        if (flags.is_list())
        {
            for (Expr const &f : flags)
            {
                if (f.is_string() && f.to_string() == "c")
                    use_compression = true;
                // Here is a place for future extensions
            }
        }
        DB_DEBUG(4, "use_compression=%d", use_compression);        

        // Parse commands
        std::vector<CmDbCommand> commands = parse_commands(e.at(1));
        CmDbQuery q;
        q.commands = parse_commands(e.at(1));
        q.timeout_usec = Nothing();
        return q;
    }
    catch (std::runtime_error &e)
    {
        DB_DEBUG(1, "Failed to parse query: %s: In", e.what());
        return Nothing();
    }
}

bool MyResponseList::receive_cancel(PartnerQueryId query_id, ServiceId service_id, View<uint8_t> data)
{
    DB_DEBUG(4, "receive_cancel: query id %llx: >>%s<<", query_id.value, std::string(data.begin(), data.end()).c_str());
    MyResponseState *qp = find_by_partner_query_id(queries, query_id);
    if (!qp)
    {
        DB_DEBUG(1, "receive_cancel: Unknown query id %llx", query_id.value);
        return false;
    }
    MyResponseState &q = *qp;

    if (q.service_id != service_id)
    {
        DB_DEBUG(1, "service id do not match");
        return false;
    }
    if (q.query_was_cancelled)
    {
        DB_DEBUG(1, "query already cancelled");
        return false;
    }
    q.query_was_cancelled = true;
    q.cancel_message = std::string(data.begin(), data.end());
    return true;
}

std::unique_ptr<CompressBase> make_compressor(bool use_compression)
{
#ifdef USE_LIB_ZSTD
    if (use_compression)
        return std::unique_ptr<CompressBase>(new Compress_zstd(9));
#endif
    return std::unique_ptr<CompressBase>(new Compress_none());
}

// Prichazi dotaz
bool MyResponseList::receive_data(PartnerQueryId query_id, ServiceId service_id, bool completed, View<uint8_t> data)
{
    DB_DEBUG(4, "receive_data: PartnerQueryId=%llx completed=%d, data=%s",
             query_id.value, completed, std::string(data.begin(), data.end()).c_str());
    MyResponseState *qp = find_by_partner_query_id(queries, query_id);
    if (!qp)
    {
        DB_DEBUG(1, "receive_data: Unknown query id %llx", query_id.value);
        return false;
    }
    MyResponseState &q = *qp;

    if (q.service_id != service_id)
    {
        DB_DEBUG(1, "service id do not match");
        return false;
    }
    if (q.query_was_completed)
    {
        DB_DEBUG(1, "query %llx was already completed", query_id.value);
        return false;
    }
    if (q.query_was_cancelled)
    {
        DB_DEBUG(1, "query %llx was already cancelled", query_id.value);
        return false;
    }
    q.query_buffer.insert(q.query_buffer.end(), data.begin(), data.end());
    if (completed)
    {
        assert(!q.query);
        assert(!q.query_was_read);
        q.query_was_completed = true;
        if (!q.is_forwarded_query)
        {
            char const *data = (char const *)q.query_buffer.data();
            q.query = parse_query(WeakString(data, data + q.query_buffer.size()), debug_name);
            if (q.query)
            {
                q.responses.resize(q.query->commands.size());
            }
            else
            {
                q.responses_error = true;
                q.error_msg = "Failed to parse";
            }
        }
    }
    return true;
}

void fill_response_buffer_if_possible(MyResponseState &q)
{
    if (!q.is_forwarded_query)
    {
        for (size_t i = 0; i < q.responses.size(); ++i)
        {
            MyReceivedResponse &r = q.responses[i];
            // If uz jsme dostali aspon jednu odpoved, ale jeste jsme ji necetli
            if (!r.was_read && (r.rows.size() > 0 || r.cmd_status != CM_DB_ROWS))
            {
                if (r.cmd_status == CM_DB_ERROR)
                {
                    // Rovnou posleme error, nedavame uz zadne radky.
                    std::vector<Expr> l;
                    l.push_back((int64_t)i);
                    l.push_back(to_string(CM_DB_ERROR));
                    l.push_back({});
                    l.push_back(r.error_message);
                    q.response_buffer.append(view_bytes(to_string(l)));
                }
                else
                {
                    for (size_t row_i = 0; row_i < r.rows.size(); ++row_i)
                    {
                        Expr const &row = r.rows[row_i];
                        bool is_last = row_i + 1 >= r.rows.size();
                        std::vector<Expr> l;
                        l.push_back((int64_t)i);
                        l.push_back(to_string(is_last ? r.cmd_status : CM_DB_ROWS));
                        l.push_back(cm_make_list(row));
                        l.push_back(r.error_message);
                        q.response_buffer.append(view_bytes(to_string(l)));
                    }
                    if (r.rows.size() == 0 && r.cmd_status == CM_DB_DONE)
                    {
                        std::vector<Expr> l;
                        l.push_back((int64_t)i);
                        l.push_back(to_string(CM_DB_DONE));
                        l.push_back(cm_make_list());
                        l.push_back(r.error_message);
                        q.response_buffer.append(view_bytes(to_string(l)));
                    }
                }
                r.rows.clear();
                r.was_read = true;
            }
        }
    }
}

bool is_last_chunk(MyResponseState const &q)
{
    if (!q.all_responses_prepared)
        return false;
    if (q.is_forwarded_query)
    {
        return true;
    }
    else
    {
        auto pred = [](MyReceivedResponse const &r){return r.cmd_status != CM_DB_ROWS && r.was_read;};
        return std::all_of(q.responses.begin(), q.responses.end(), pred);
    }
}

void MyResponseList::erase_response(QueryId query_id)
{
    auto it = std::remove_if(queries.begin(), queries.end(), [=](MyResponseState &r){return r.user_query_id == query_id;});
    queries.erase(it, queries.end());
};

// Posilame nasi odpoved
void MyResponseList::generate_data_to_send(OutputBufferSimple &out)
{
    size_t max_size = out.get_max_size();

    // pricitame +1 za pridavany znak 't' (kvuli zpetne kompatibilite)
    if (max_size <= query_header_length() + 1)
        return;
    max_size -= query_header_length() + 1;

    for (MyResponseState &q : queries)
    {
        if (q.query_was_cancelled)
            continue;

        if (q.responses_error)
        {
            // size_t len = std::min(q.error_msg.size(), max_size);
            std::vector<uint8_t> msg;
            // msg.push_back('E');
            push_hex_number(msg, q.service_id.value, 4);
            push_hex_number(msg, q.partner_query_id.value, 16);
            // msg.insert(msg.end(), q.error_msg.begin(), q.error_msg.begin() + len);

            ssize_t len = out.try_add_message('E', 'E', msg, view_bytes(q.error_msg), 0);
            if (len >= 0)
            {
                erase_response(q.user_query_id);
                return;
            }
        }

        if (!q.query_was_completed)
            continue;

        fill_response_buffer_if_possible(q);
        ByteBuffer &buffer = q.response_buffer;
        
        if (!buffer.empty())
        {
            // Pokud is_last_chunk() == true, pak uz buffer obsahuje celou odpoved.
            // bool whole_response_completed = is_last_chunk(q) && max_size >= buffer.size();
            // size_t size = std::min(max_size, buffer.size());
            // DB_DEBUG(3, "Generating msg: msg_max=%zu, to_send=%zu, all_responses_prepared=%d, is_forwarded_query=%d, whole_response_completed=%d",
            //          size, q.response_buffer.size(), q.all_responses_prepared, q.is_forwarded_query, whole_response_completed);

            std::vector<uint8_t> msg;
            // msg.push_back(whole_response_completed ? 'R' : 'r');
            push_hex_number(msg, q.service_id.value, 4);
            push_hex_number(msg, q.partner_query_id.value, 16);

            // Tohle tam davame z duvodu kompatibility se starsimi
            // verzemi protokolu
            if (!q.first_response_was_sent)
                msg.push_back('t');
            
            // msg.insert(msg.end(), buffer.begin(), buffer.begin() + size);

            ssize_t bytes_sent = out.try_add_message('r', is_last_chunk(q) ? 'R' : 'r',
                                                     msg, view_bytes(buffer.begin(), buffer.end()),
                                                     1);
            if (bytes_sent >= 0)
            {
                assert(bytes_sent > 0);
                bool whole_response_completed = is_last_chunk(q) && (size_t)bytes_sent == buffer.size();
                
                q.first_response_was_sent = true;
                buffer.advance(bytes_sent);

                if (whole_response_completed)
                    erase_response(q.user_query_id);

                DB_DEBUG(4, "generate_data_to_send: %s", std::string(msg.begin(), msg.end()).c_str());
                return;
            }
        }
    }
}

std::map<QueryId, std::string> MyResponseList::get_cancelled_queries()
{
    std::map<QueryId, std::string> result;
    for (MyResponseState &q : queries)
    {
        if (q.query_was_cancelled)
            result.insert({q.user_query_id, q.cancel_message});
    }
    for (auto const &p : result)
        erase_response(p.first);
    return result;
}

void MyResponseList::disconnect()
{
    for (MyResponseState &q : queries)
    {
        if (!q.query_was_cancelled)
        {
            q.query_was_cancelled = true;
            q.cancel_message = "Partner disconnected";
        }
    }
}

size_t MyResponseList::size() const
{
    return queries.size();
}

std::vector<std::string> MyResponseList::get_debug() const
{
    std::vector<std::string> lines;
    for (MyResponseState const &q : queries)
    {

        char const *type = q.is_forwarded_query ?  "ForwardedQuery" : "Query";
        lines.push_back(format("%s user_qid=%lld, partner_qid=%lld, q_received=%d, q_read=%d q_cancelled=%d: n_rows=%zu",
                               type,
                               q.user_query_id.value,
                               q.partner_query_id.value,
                               q.query_was_completed,
                               q.query_was_read,
                               q.query_was_cancelled,
                               q.rows_added));
    }
    return lines;
}
