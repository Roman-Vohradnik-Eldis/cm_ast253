#ifndef CM_QUERY_LIST_HH_
#define CM_QUERY_LIST_HH_

#include "cm_db_query_common.hh"
#include "cm_db_query_state.hh"
#include "cm_send_buffer_simple.hh"

// Nase dotazy, ktere posilame partnerum a dostavame na ne odpovedi.
//
// Uzivatel ma dva zpusoby, jak zadat dotaz:
//
// (1) Bud pomoci add_new_query(): Pak MyQueryList bude odpovedi
// postupne parsovat, a budeme je bude dostavat pomoci
// get_new_responses().
//
// (2) Nebo pomoci begin_forwarded_query(): Pak musime dotaz
// postupne naplnit pomoci add_forwarded_query_data(). Pak
// odpovedi bude dostavat nenaparsovane pomoci
// get_forwarded_responses().
//
class MyQueryList
{
    std::vector<MyQueryState> queries;
    std::string debug_name;

public:
    MyQueryList(std::string debug_name);

    // Tohle pouzivame pri hledani dotazu, jakemu partnerovi patri
    bool has_query_id(QueryId query_id);

    // Zrusi dotaz.
    //
    // generate_callback = false: Nic od tohoto dotazu uz do callbacku neprijde.
    //
    // generate_callback = true: Do callbacku prijdou nedokoncene
    // prikazy jako chybove. V pripade nedokoncene forwarded_query
    // prijde do callbacku abort.
    void cancel_query(QueryId query_id, WeakString reason, bool generate_callback);
    
    // Tohle pouziva uzivatel
    void add_new_query(QueryId user_id,
                       ServiceId service_id,
                       DbService service_name,
                       CmDbQuery const &query,
                       int64_t time_now);

    // Tohle pouziva router. Nejdrive zahaji dotaz pomoci
    // `begin_forwarded_query`, pak tam pridava data pomoci
    // `add_forwarded_query_data`.
    void begin_forwarded_query(QueryId user_id, ServiceId service_id, DbService service_name);

    // Tohle pouziva router
    void add_forwarded_query_data(QueryId user_id, bool was_completed, View<uint8_t> data);

    // Tohle jde uzivatlovi do callbacku
    std::vector<CmDbReceivedResponse> get_new_responses(size_t limit); // limit 0 = unlimited

    // Tohle jde uzivatlovi do callbacku. Je to zprava typu r, R nebo E.
    std::vector<DbMessage> get_forwarded_responses();

    // Posila se dotaz.
    // max_length: delka zprava i s hlavickou
    void generate_data_to_send(OutputBufferSimple &out);

    // Tohle volame, kdyz prijemem zpravu od partnera. Vrati true,
    // pokud mame nejakou novou zmenu, tj. odpoved, kterou si muze
    // uzivatel vyzvednout. `kind` je 'r', 'R' nebo 'E'.
    bool receive_data(PartnerQueryId query_id, ServiceId service_id, View<uint8_t> data, char kind);

    // Tohle pouzivame, kdyz se partner odpoji.  U vsechny aktivnich
    // dotazu to nastavi chybovou odpoved, kterou si muze uzivatel
    // vyzvednout.
    void disconnect();

    // Number of active queries
    size_t size() const;

    // Vrati true, pokud nastal timeout u nejakeho dotazu. Pokud ano,
    // tak typicky jsou k dipozici data k odeslani a odpoved (ze
    // nastal timeout) do callbacku.
    bool check_timeout(int64_t time_now);

    // V kolik hodin kontrolovat dalsi timeout. Nothing: neni treba
    Maybe<int64_t> get_next_wakeup_time();

private:
    void remove_finished_queries();
};

#endif // CM_QUERY_LIST_HH_
