#include "cm_db_query_common.hh"

void add_to_query_state(std::vector<MyReceivedResponse> &responses, size_t cmd_id, CmDbCommandStatus status, std::vector<Expr> const &rows, std::string const &err_msg)
{
    cms_ns_if_print("cmdb", 4, "add_received_response: status=%s, cmd_id=%zu, rows=%s", to_string(status).c_str(), cmd_id, to_string(Expr(rows)).c_str());
    if (cmd_id >= responses.size())
    {
        cms_ns_if_print("cmdb", 1, "add_received_response: cmd_id = %zu >= query_size = %zu", cmd_id, responses.size());
        return;
    }
    MyReceivedResponse &r = responses[cmd_id];
    if (r.cmd_status != CM_DB_ROWS)
    {
        cms_ns_if_print("cmdb", 1, "add_received_response: Command %zu already completed: ", cmd_id);
        return;
    }
    if (r.cmd_status != status)
    {
        r.was_read = false;
        r.cmd_status = status;
    }
    if (!rows.empty())
    {
        r.was_read = false;
        r.rows.insert(r.rows.end(), rows.begin(), rows.end());
    }
    r.error_message = err_msg;
}


std::ostream &operator<<(std::ostream &o, MyReceivedResponse const &r)
{
    return o << "[was_read=" << r.was_read
             << ", cmd_status=" << r.cmd_status
             << ", rows=" << Expr(r.rows)
             << ", error_message=" << Expr(r.error_message)
             << "]";
}

std::ostream &operator<<(std::ostream &o, std::vector<MyReceivedResponse> const &responses)
{
    o << "{";
    for (auto const &c : responses)
        o << " " << c;
    o << " }";
    return o;
}
