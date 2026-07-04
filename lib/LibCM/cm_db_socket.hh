#ifndef CM_DB_SOCKET_HH_
#define CM_DB_SOCKET_HH_

#include "cm_base.hh"
#include "cm_thread.hh"

struct CmDbSocketResponse
{
    std::vector<Expr> rows;
    CmDbCommandStatus status = CM_DB_ROWS;
    std::string error_message;
};

class CmDbSocket
{
    CMSMutex mutex;
    CMSCondition cond;
    bool was_stopped = false;

    CmDbConnection *db_connection;
    DbService service_name;

    // Soucasny dotaz
    Maybe<QueryId> query_id;    
    std::vector<CmDbSocketResponse> responses;

public:
    CmDbSocket(CmDbConnection *db_conn, DbService service);
    ~CmDbSocket();

    // Posle zadany dotaz, ihned vrati.
    bool send_query(CmDbQuery q);
    
    // Blokuje, nez se dokonci posledni send_query. Pokud nekdo spusti
    // v prubehu cekani novy send_query, chovani je nedefinovane.
    void wait_all();

    // Jako wait_all. Pokud vyprsi timeout, vrati false
    bool wait_all_with_timeout(int64_t usec);

    // Vrati true, pokud jsme uz na predchozi dotaz dostali kompletni
    // odpoved a zaroven vsechny jeho prikazy uspely.
    bool all_succeeded();

    std::vector<Expr> get_rows(size_t command_id);

    // Pokud jich je vic, je viceradkova
    std::string get_error_message();

    void receive_responses(std::vector<CmDbReceivedResponse> const &responses);

    void close();

private:
    void process_response(CmDbReceivedResponse const &resp);
    bool internal_is_completed();
};

#endif // CM_DB_SOCKET_HH_
