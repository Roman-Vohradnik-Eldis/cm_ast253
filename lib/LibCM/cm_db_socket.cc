#include "cm_db_socket.hh"
#include "cm_utils.hh"

CmDbSocket::~CmDbSocket()
{
    close();
}

CmDbSocket::CmDbSocket(CmDbConnection *db_conn, DbService service)
    : db_connection(db_conn), service_name(service)
{
}

void CmDbSocket::close()
{
    CMSScopedLock lock(mutex);
    was_stopped = true;
    cond.Broadcast();
}

bool CmDbSocket::send_query(CmDbQuery q)    
{
    CMSScopedLock lock(mutex);
    responses.clear();
    size_t length = q.commands.size();
    query_id = db_connection->DbSendQuery(service_name, std::move(q));
    if (query_id)
    {
        responses.resize(length);
        return true;
    }
    return false;
}

bool CmDbSocket::internal_is_completed()
{
    for (CmDbSocketResponse const &r : responses)
        if (r.status == CM_DB_ROWS)
            return false;
    return true;
}

void CmDbSocket::wait_all()
{
    CMSScopedLock lock(mutex);
    if (!query_id)
        return;
    while (!was_stopped && !internal_is_completed())
        cond.Wait(&mutex);
}

std::vector<Expr> CmDbSocket::get_rows(size_t command_id)
{
    CMSScopedLock lock(mutex);
    return responses.at(command_id).rows;
}

bool CmDbSocket::wait_all_with_timeout(int64_t usec)
{
    CMSScopedLock lock(mutex);
    if (!query_id)
        throw std::logic_error("Query was not sent");
    int64_t time_start = get_current_time_usec();
    while (!was_stopped && !internal_is_completed())
    {
        cond.WaitTime(&mutex, usec);
        int64_t time_now = get_current_time_usec();
        if (!query_id)
            throw std::logic_error("Query changed while waiting");
        if (time_start + usec < time_now)
        {
            for (size_t i = 0; i < responses.size(); ++i)
            {
                CmDbSocketResponse const &r = responses[i];
                cms_ns_if_print("dbsocket", 1, "timeout: query_id=%llx, status=%s errmsg=%s rows=%s",
                                query_id->value,
                                to_string(r.status).c_str(),
                                r.error_message.c_str(),
                                to_string(r.rows).c_str());
            }
            return false;
        }
    }
    return true;
}

bool CmDbSocket::all_succeeded()
{
    CMSScopedLock lock(mutex);
    for (CmDbSocketResponse const &r : responses)
        if (r.status != CM_DB_DONE)
            return false;
    return true;
}

// mame zamknuto
void CmDbSocket::process_response(CmDbReceivedResponse const &resp)
{
    if (resp.cmd_id >= responses.size())
    {
        cms_ns_if_print("cmdb", 1, "CmDbSocket cmd_id out of range");
        return;
    }
    CmDbSocketResponse &r = responses[resp.cmd_id];
    if (r.status != CM_DB_ROWS)
    {
        cms_ns_if_print("cmdb", 1, "CmDbSocket unexpected response");
        return;
    }
    r.status = resp.cmd_status;
    r.rows.insert(r.rows.end(), resp.rows.begin(), resp.rows.end());
    r.error_message = resp.error_message;
}

std::string CmDbSocket::get_error_message()
{
    CMSScopedLock lock(mutex);
    std::string msg;
    for (CmDbSocketResponse const &r : responses)
    {
        if (!r.error_message.empty())
        {
            if (!msg.empty())
                msg += "\n";
            msg += r.error_message;
        }
    }
    return msg;
}

void CmDbSocket::receive_responses(std::vector<CmDbReceivedResponse> const &responses)
{
    CMSScopedLock lock(mutex);
    bool changed = false;
    if (query_id)
    {
        for (CmDbReceivedResponse const &r : responses)
        {
            if (r.query_id == *query_id)
            {
                process_response(r);
                changed = true;
            }
        }
    }
    if (changed)
        cond.Broadcast();
}
