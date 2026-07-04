#ifndef CM_DB_TYPES_HH_
#define CM_DB_TYPES_HH_

#include "expr.hh"
#include <stdint.h>
#include <inttypes.h>

struct QueryId
{
    long long value;

    QueryId() = delete;
};

inline bool operator==(QueryId a, QueryId b) { return a.value == b.value; }
inline bool operator!=(QueryId a, QueryId b) { return a.value != b.value; }
inline bool operator<(QueryId a, QueryId b) { return a.value < b.value; }
inline std::ostream &operator<<(std::ostream &out, QueryId a) { return out << a.value; }

// Struktury pro uzivatele
//////////////////////////////////////////////////////////////////////

// Integer values are sent to the partners, do not change their order
enum CmDbCommandStatus
{
    CM_DB_DONE = 0,
    CM_DB_ROWS = 1,
    CM_DB_ERROR = 2,
};

struct CmDbCommand
{
    std::string cmd;
    std::vector<Expr> args;
};

bool cm_is_valid_service_name(WeakString service_name);

// DbService je identifikován jménem služby. To je většinou krátké
// (několikapísmenné). Typicky tuto strukturu budeme předávat
// hodnotou.
struct DbService
{
    std::string name;

    char const *c_str() const { return name.c_str(); }

    DbService(const char *name)
        : name(name)
    {
        if (!cm_is_valid_service_name(name))
            throw std::runtime_error("Invalid service name");        
    }
    
    DbService(std::string name)
        : name(name)
    {
        if (!cm_is_valid_service_name(name))
            throw std::runtime_error("Invalid service name");
    }
};

inline std::ostream &operator<<(std::ostream &out, DbService const &s) { return out << s.name; }

inline bool operator<(DbService const &a, DbService const &b) {return a.name < b.name;}
inline bool operator==(DbService const &a, DbService const &b) {return a.name == b.name;}
inline bool operator!=(DbService const &a, DbService const &b) {return a.name != b.name;}

// Tohle uzivatel dostane v CmChanges, kdyz mu nekdo odpovi na doz.
//
// service_name: Jmeno service, s cim bylo poslan dotaz.
// Dotaz mel N prikazu CmDbCommand: pak 0 <= cm_id < N
// error_message: pouzito v pripade cmd_status == CM_DB_ERROR
struct CmDbReceivedResponse
{
    QueryId query_id;
    size_t cmd_id;
    DbService service_name;
    CmDbCommandStatus cmd_status;
    std::vector<Expr> rows;
    std::string error_message;
    bool is_query_completed;
};

// Tohle uzivatel predava do LibCM, kdyz chce odpovedet na dotaz
struct CmDbResponse
{
    QueryId query_id;
    size_t cmd_id;
    CmDbCommandStatus cmd_status;
    std::vector<Expr> rows;
    std::string error_message;
};

// Tohle uzivatel predava do LibCM, aby se dotaz poslal
struct CmDbQuery
{
    std::vector<CmDbCommand> commands;
    Maybe<int64_t> timeout_usec;

    CmDbQuery(std::vector<CmDbCommand> cmds, Maybe<int64_t> timeout_usec)
        : commands(std::move(cmds)), timeout_usec(timeout_usec)
    {}

    CmDbQuery()
    {}
};

// Tohle uzivatel dostane v CmChanges, kdyz mu nekdo posle dotaz.
struct CmDbReceivedQuery
{
    QueryId query_id;
    DbService service;
    std::vector<CmDbCommand> commands;
};

// Pro forwardovani zprav
struct DbMessage
{
    char kind;
    DbService service;
    QueryId query_id;
    std::vector<uint8_t> data;
};

struct DbServiceParams
{
    // forwarded = false: Odpoved se naparsuje a jde do callbacku jako CmDbReceivedResponse
    // forwarded = true: Odpoved jde do callbacku jako DbMessage
    bool forward;

    // Vsem partnerum se posle informace, ze tuto sluzby poskytujeme
    bool external;

    DbServiceParams()
        : forward(false), external(true)
    {}
};

inline CmDbResponse make_error_response(QueryId query_id, size_t cmd_id, std::string err_msg)
{
    return {query_id, cmd_id, CM_DB_ERROR, {}, std::move(err_msg)};
}

inline CmDbResponse make_done_response(QueryId query_id, size_t cmd_id, std::vector<Expr> rows)
{
    return {query_id, cmd_id, CM_DB_DONE, std::move(rows), ""};
}

inline CmDbResponse make_rows_response(QueryId query_id, size_t cmd_id, std::vector<Expr> rows)
{
    return {query_id, cmd_id, CM_DB_ROWS, std::move(rows), ""};
}

inline bool operator==(DbServiceParams const &a, DbServiceParams const &b)
{ return a.forward == b.forward; }

CmDbCommandStatus read_command_status(std::string const &s);

std::ostream &operator<<(std::ostream &o, DbMessage const &msg);

std::ostream &operator<<(std::ostream &o, CmDbQuery const &q);

std::ostream &operator<<(std::ostream &o, CmDbReceivedQuery const &q);

std::ostream &operator<<(std::ostream &o, CmDbCommandStatus s);

std::ostream &operator<<(std::ostream &o, CmDbReceivedResponse const &r);

std::ostream &operator<<(std::ostream &o, CmDbResponse const &r);

#endif // CM_DB_TYPES_HH_
