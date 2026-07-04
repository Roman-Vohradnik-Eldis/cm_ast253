#include "cm_db_types.hh"

CmDbCommandStatus read_command_status(std::string const &s)
{
    if (s == "ROWS") return CmDbCommandStatus::CM_DB_ROWS;
    if (s == "ERROR") return CmDbCommandStatus::CM_DB_ERROR;
    if (s == "DONE") return CmDbCommandStatus::CM_DB_DONE;
    throw std::runtime_error("Invalid command status");
}

std::ostream &operator<<(std::ostream &o, DbMessage const &msg)
{
    o << "[msg kind=" << msg.kind
      << " service=" << msg.service
      << " id=" << format("%llx", msg.query_id.value)
      << " len=" << msg.data.size()
      << " data=" << std::string(msg.data.begin(), msg.data.end())
      << "]";
    return o;
}

std::ostream &operator<<(std::ostream &o, CmDbQuery const &q)
{
    o << "[query";
    o << " timeout=" << (q.timeout_usec ? format("%lld msec", (long long)*q.timeout_usec / 1000) : "No");
    o << ":";
    for (CmDbCommand const &c : q.commands)
    {
        o << " " << c.cmd << " " << c.args;
    }
    o << "]";
    return o;
}

std::ostream &operator<<(std::ostream &o, CmDbReceivedQuery const &q)
{
    o << "[query service=" << q.service
      << ", id=" << format("%llx", q.query_id.value)
      << ":";
    for (CmDbCommand const &c : q.commands)
    {
        o << " " << c.cmd << " " << c.args;
    }
    o << "]";
    return o;
}

std::ostream &operator<<(std::ostream &o, CmDbCommandStatus s)
{
    switch (s)
    {
    case CM_DB_ROWS: return o << "ROWS";
    case CM_DB_ERROR: return o << "ERROR";
    case CM_DB_DONE: return o << "DONE";
    default: return o << "invalid";
    }
}

std::ostream &operator<<(std::ostream &o, CmDbReceivedResponse const &r)
{
    o << "[response id=" << format("%llx", r.query_id.value)
      << ", cmd_id=" << r.cmd_id
      << ", cmd_status=" << r.cmd_status
      << ", err='" << r.error_message << "'"
      << ":";
    for (Expr const &e : r.rows)
    {
        o << " " << e;
    }
    o << "]";
    return o;
}

std::ostream &operator<<(std::ostream &o, CmDbResponse const &r)
{
    o << "[response id=" << format("%llx", r.query_id.value)
      << ", cmd_id=" << r.cmd_id
      << ", cmd_status=" << r.cmd_status
      << ", err='" << r.error_message << "'"
      << ":";
    for (Expr const &e : r.rows)
    {
        o << " " << e;
    }
    o << "]";
    return o;
}

bool cm_is_valid_service_name(WeakString service_name)
{
    if (service_name.empty())
        return false;
    if (!isalpha(service_name[0]) && service_name[0] != '_')
        return false;
    for (char c : service_name)
    {
        if (!isalnum(c) && c != '_')
            return false;
    }
    return true;
}
