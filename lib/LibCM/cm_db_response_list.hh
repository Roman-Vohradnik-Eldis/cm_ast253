#ifndef CM_RESPONSE_LIST_HH_
#define CM_RESPONSE_LIST_HH_

#include "cm_db_response_status.hh"
#include "cm_db_types.hh"
#include "cm_send_buffer_simple.hh"

// Od partner nam prijde dotaz.
// My ho predame do callbacku uzivatelovi.
// Uzivatel pak pridava odpovedi a my je posilame.
//
class MyResponseList
{
    std::list<MyResponseState> queries;
    std::string debug_name;
    
public:
    MyResponseList(std::string dbg_name);
    
    bool has_query(PartnerQueryId query_id);
    bool has_query_id(QueryId query_id);
    std::set<QueryId> get_all_query_ids();

    // Tohle pouzivame, kdyz se partner odpoji.  U vsechny aktivnich
    // dotazu to nastavi, ze response was cancelled, coz jde pak do
    // callbacku.
    void disconnect();
    
    // Dotaz lze pridat dvema zpusoby:
    //
    // (1) Bud zada forward = false, pak musi odpovedi zadavat pomoci
    // add_received_response().
    //
    // (2) Nebo zada forward = true, pak musi odpovedi zadavat pomoci
    // add_forwarded_response.
    
    // Od partnera prijde novy dotaz, timto se na neho pripravime.
    // Dotaz identifikujeme jako user_id, ale posilame ho s partner_id.
    void create_new_query(QueryId user_id,
                          PartnerQueryId partner_id,
                          ServiceId service_id,
                          DbService service_name,
                          bool forward,
                          bool compress);

    // Do callbacku.  limit 0 = unlimited
    std::vector<CmDbReceivedQuery> get_new_queries(size_t limit);

    // Do callbacku: Jsou to zpravy typu q, Q.
    std::vector<DbMessage> get_queries_to_forward();

    // Do callbacku: QueryId -> CancelMessage
    std::map<QueryId, std::string> get_cancelled_queries();

    // Posleme odpoved. Tohle muzeme pouzit jen pro zpravy, u kterych
    // je forward=true.  kind je: 'r', 'R' nebo 'E'.
    void send_forwarded_response(QueryId query_id, View<uint8_t> data, char kind);

    // Posleme odpoved. Tohle muzeme pouzit jen pro zpravy, u kterych je forward=false
    void send_response(QueryId query_id,
                       size_t cmd_id,
                       CmDbCommandStatus status,
                       std::vector<Expr> const &rows,
                       std::string const &err_msg);

    // Vrati true, pokud jsou k dispozici nejake zmeny do callbacku nebo k odeslani
    bool receive_data(PartnerQueryId query_id, ServiceId service_id, bool completed, View<uint8_t> data);

    // Vrati true, pokud jsou k dispozici nejake zmeny do callbacku nebo k odeslani
    bool receive_cancel(PartnerQueryId query_id, ServiceId service_id, View<uint8_t> data);

    // Posilame odpoved partnerovi
    void generate_data_to_send(OutputBufferSimple &out);

    size_t size() const;

    std::vector<std::string> get_debug() const;

private:
    void erase_response(QueryId);
};

#endif // CM_RESPONSE_LIST_HH_
