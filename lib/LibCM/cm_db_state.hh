#ifndef CM_DB_STATE_HH_
#define CM_DB_STATE_HH_

#include "cm_db_query_list.hh"
#include "cm_db_response_list.hh"
#include "cm_db_types.hh"
#include "cm_send_buffer_simple.hh"
#include "cm_db_timer_list.hh"

#include <list>
#include <map>
#include <memory>
#include <set>

// 1. Partner poskytuje sluzby a my je muzeme vyuzivat
// 2. My posktujeme sluzby a partner je muze vyuzivat
// 3. Partner nám posílá dotazy a my mu odpovídáme
// 4. My posíláme partnerovi dotazy a on nám odpovídá
//
struct CmDbPartner
{
    // If id is not set, partner is used for self queries.
    Maybe<unsigned> id;

    // Uzivatel zavolal disconnect_partner(). Jeste ho stale drzime v
    // pameti, dokud si uzivatel nevyzvedne vsechny dotazy a odpovedi.
    bool is_disconnected = false;

    // Tady jsou ty, co nam poslal partner
    std::map<DbService, ServiceId> partners_accepted_services;

    // Tady jsou ty, co jsme my poslali partnerovi
    std::map<DbService, ServiceId> my_accepted_services;

    ServiceId my_service_id_counter{rand() % 1000};

    MyQueryList my_query_list;
    MyResponseList my_response_list;

    // Zajistujume kompresi mezi partnerem a nami.
    bool recv_use_control_character;
    SendBufferSimple send_buffer;
    RecvBufferState recv_buffer;

    // @param debug_name: LibCM name for debugging
    // @param partner_name: For debugging
    CmDbPartner(Maybe<unsigned> i,
                std::string debug_name,
                bool recv_use_control_character,
                SendBufferSimpleSettings const &buff_settings)
        : id(i),
          my_query_list(debug_name),
          my_response_list(debug_name),
          recv_use_control_character(recv_use_control_character),
          send_buffer(buff_settings),
          recv_buffer({})
    {}
};

enum CmDbMasterSlavePolicy
{
    CM_DB_SEND_TO_MASTER,
    CM_DB_SEND_TO_ANY,
};

struct CmDbHasCallback
{
    bool value;

    explicit operator bool() const { return value; }
};

struct CmDbHasData
{
    bool value;

    explicit operator bool() const { return value; }
};

struct DbAvailableServiceParams
{
    // Kdyz posleme dotaz, tak se nam vrati v callbacku jako forwarded query
    bool is_forwarded_locally = false;

    // Kdyz posleme dotaz, tak se nam vrati v callbacku
    bool is_provided_locally = false;

    // Kdyz posleme dotaz, tak se posle po siti partnerovi
    bool is_provided_by_partner = false;
};

struct PartnerParams
{
    bool recv_use_control_character;

    SendBufferSimpleSettings send_settings;
};

inline bool operator==(DbAvailableServiceParams const &a, DbAvailableServiceParams const &b)
{
    return
        a.is_provided_by_partner == b.is_provided_by_partner &&
        a.is_provided_locally == b.is_provided_locally;
}

class CmDbState
{
    std::list<CmDbPartner> partners;
    QueryId my_query_counter;

    // Tady jsou ty, co sem uzivatel dal pomoci accept_service
    std::map<DbService, DbServiceParams> my_services;

    // Tady jsou posledni services, co sly do callbacku.
    std::map<DbService, DbAvailableServiceParams> all_partners_accepted_services;

    std::string debug_name;

    CmDbMasterSlavePolicy master_slave_policy = CM_DB_SEND_TO_ANY;
    Maybe<unsigned> master_partner;

    // Odpovedi na nase dotazy, u kterych nebyla dostupna sluzba.
    std::vector<CmDbReceivedResponse> responses_from_nonexistant_services;

    CmDbTimerList timer_list;

public:
    CmDbState(WeakString dbg_name);

    void set_master_slave_policy(CmDbMasterSlavePolicy msv_policy);

    // Tohle vola uzivatel knihovny
    void accept_service(DbService s, DbServiceParams forward);

    // services : ServiceName -> (Forward:True/Fales)
    void set_accepted_services(std::map<DbService, DbServiceParams> const &services);

    // Tohle vola uzivatel knihovny
    // Pokud nikdo neposkytuje zadanou sluzbu, tak nic nedela a vrati Nothing.
    Maybe<QueryId> send_query_maybe(DbService service, CmDbQuery const &q, int64_t time_now);

    // Tohle vola uzivatel knihovny
    // Pokud nikdo neposkytuje zadanou sluzbu, tak vygeneruje callback s errorem
    QueryId send_query(DbService service, CmDbQuery const &q, int64_t time_now);

    // Tohle vola uzivatel knihovny
    void send_response(CmDbResponse response);

    // Tohle vola uzivatel knihovny
    Maybe<QueryId> begin_forwarded_query(DbService service);
    void add_to_forwarded_query(DbMessage msg);

    // Tohle vola uzivatel knihovny
    void add_forwarded_response(DbMessage msg);

    void cancel_query(QueryId query_id, WeakString reason, bool gen_callback);

    // Aplikace musi po poslani dotazu zavolat get_next_wakeup_time,
    // nastavit si nejaky svuj casovac a pak v zadany cas zavolat
    // wakeup().
    Maybe<int64_t> get_next_wakeup_time();

    // Vrati true, pokud je potreba zavolat CB nebo Send
    bool wakeup(int64_t time_now = get_current_time_usec());

    size_t query_count() const;

    size_t response_count() const;

    // Tohle jde do CmChanges
    std::vector<CmDbReceivedQuery> get_received_queries();
    std::vector<CmDbReceivedResponse> get_received_responses();
    Maybe<std::map<DbService, DbAvailableServiceParams>> get_accepted_services_change();
    std::vector<DbMessage> get_forwarded_responses();
    std::vector<DbMessage> get_forwarded_queries();
    std::map<QueryId, std::string> get_cancelled_queries();

    // @partner_name: Used for debugging
    void connect_partner(unsigned partner_id, PartnerParams const &params);
    void disconnect_partner(unsigned partner_id);

    void set_master(unsigned partner_id);
    void unset_master();

    // Volame, kdyz prijde textova zprava od partnera.  Vrati true,
    // pokud jsou k dispozici nejake zmeny do CmChanges.  Jinak vrati
    // false. Partner musi existovat (tj. musi byt zavolano
    // connect_partner).  Pri chybe to jen vypise chybu.
    bool receive_text_data(unsigned partner_id, View<uint8_t> data);

    // Vygenerujeme textovou zpravu pro partnera.  Pokud nic
    // nepotrebujeme poslat, vrati "".  Partner musi existovat
    // (tj. musi byt zavolano connect_partner).
    //
    // Tohle bychom meli zavolat pro vsechny partnery pokazde, kdyz
    // se vola kterakoliv jina funkce z CmDbState.
    std::vector<uint8_t> generate_data_to_send(unsigned partner_id, Maybe<size_t> max_length);

    std::vector<std::string> get_debug() const;

private:
    bool receive_partner_data(CmDbPartner &p, View<uint8_t> orig_data);
    void process_self_messages();
    CmDbPartner *find_partner(unsigned id);
    CmDbPartner *find_destination_partner(DbService service);
    void generate_accepted_services_to_send(CmDbPartner &p, OutputBufferSimple &out);
    DbServiceParams create_or_update_service_params(DbService service_name, bool forward);

    bool receive_msg_accept(CmDbPartner &p, View<uint8_t> orig_data, View<uint8_t> data);
    bool receive_msg_reject(CmDbPartner &p, View<uint8_t> orig_data, View<uint8_t> data);
    bool receive_msg_full(CmDbPartner &p, char kind, View<uint8_t> orig_data, View<uint8_t> data);
};

#endif // CM_DB_STATE_HH_
