#ifndef CM_DB_QUERY_STATE_HH_
#define CM_DB_QUERY_STATE_HH_

#include "cm_db_query_common.hh"
#include "cm_string_buffer.hh"
#include "expr_parser.hh"
#include "cm_view.hh"
#include "compress_base.hh"
#include "cm_recv_buffer_state.hh"
#include "cm_db_response_buffer.hh"

// Terminologie:
//
// query cancelled: Zrusili jsme dotaz (v prubehu posilani dotazu nebo
// v prubehu prijimani odpovedi).
//
// query aborted: Server zrusil dotaz.

enum class QueryStatus
{
    Running, // Dotaz jeste nemam kompletni, a uz ho posilam
    Prepared, // Uz nic nebudu do dotazu pridavat, mozna ale neni uplne odeslany
    NeedSendCancel, // Chci poslat cancel (je jedno, co jsem doted poslal)
    CancelWasSent, // Cekam chvili, pak vse zahodim
    Aborted, // Partner poslal abort. Nic uz mu nebudeme posilat.
};

enum class ResponseStatus
{
    Running, // Cekam na radky, davam je do callbacku. (nebo inicialni stav)
    Completed, //  Odpoved je kompletni, cekam, az si uzivatel vyzvedne
    Aborted, // Odpoved je kompletni (chybova), cekam, az si ji uzivatel vyzvedne.
    Cancelled, // Odpoved uz me nezajima. V pripade is_forwaded to
               // znamena, ze uzivatel uz si vse vyzvedl.
};

// Ja jsem Client, a posilam dotaz na Server
//
// Kdyz chci posilat novy dotaz, tak se tato struktura vytvori.  Pak
// existuje do te doby, nez prijmeme celou odpoved a uzivatel si je
// celou nevyzvedne. Pak hned zanika.
//
// Kdyz chci tento dotaz zrusit (protoze chce uzivatel nebo kvuli
// chybe), tak nastavim priznak was_cancelled a poslu Abort
// Serveru. Drzim MyQueryState jeste treba minutu, protoze jeste
// muzeme stale dostavat odpoved od serveru (kterou zahazujeme).
struct MyQueryState
{
    MyQueryState(QueryId u_id, ServiceId sid, DbService service_name)
        : user_query_id(u_id),
          service_id(sid),
          service_name(service_name)
    {}
    
    // KONFIGURACE
    //////////////////////////////////////////////////
    
    // user_query_id je zde zaroven to query id, co se posila partnerovi
    QueryId user_query_id;
    ServiceId service_id;
    DbService service_name;
    bool is_forwarded_query = false;
    int64_t time_created;
    int64_t time_timeout;

    // Tohle muze nastavit kdykoliv Client.  Pokud jsme nezacali
    // posilat dotaz, vsechno zahodime.  Pokud teprve posilame dotaz,
    // tak prestaneme a posleme abort. Pokud uz prijimame odpoved, tak
    // posleme abort a nove prichozi zpravy zahazujeme.
    bool something_was_sent = false;
    QueryStatus query_status = QueryStatus::Running;
    std::string cancel_message;

    // POSILANI DOTAZU
    //////////////////////////////////////////////////

    // query jako string, co nam zbyva poslat. Pokud to jde, tak vzdy odesilame.
    ByteBuffer query_buffer;

    // Jestli budeme jeste neco pridavat do query_buffer.  Az bude
    // tohle true && query_buffer.empty(), tak to znamena, ze jsme
    // dotaz odeslali cely a cekame odpoved.
    // bool query_was_prepared = false;
    
    // Az bude query_buffer prazdna & query_was_prepade == true,
    // nastavime query_was_sent na true, a cekame na odpoved.
    // bool query_was_sent = false;

    // PRIJIMANI ODPOVEDI
    //////////////////////////////////////////////////

    // Prichozi data ukladame do `response_buffer`. Pokud je
    // is_forwarded_query == false, tak to postupne parsujeme na radky
    // a ty premistujeme do received_responses. Pokud je
    // is_forwarded_query == true, tak data z `response_buffer`
    // rovnou bereme a davame do callbacku.
    ByteBuffer response_buffer;

    // Stav cele odpovedi.
    ResponseStatus response_status = ResponseStatus::Running;

    std::string abort_message;

    // Pouzito pouze pro is_forwarded_query == false.  Je to
    // naalokovane na pocet prikazu v poslanem dotazu. Sem davame
    // naparsovane radky z response_buffer. Odtud to pak predavame do
    // callbacku.
    std::vector<MyReceivedResponse> received_responses;
    ExprParser expr_parser;

    unsigned bytes_sent = 0;
    unsigned bytes_received = 0;
    unsigned messages_received = 0;
};

inline MyQueryState make_forwarded_query(QueryId u_id, ServiceId sid, DbService service_name)
{
    MyQueryState q(u_id, sid, service_name);
    q.is_forwarded_query = true;
    return q;
}

inline MyQueryState make_new_query(QueryId u_id,
                                   ServiceId sid,
                                   DbService service_name,
                                   WeakString b,
                                   size_t n_commands,
                                   int64_t timeout,
                                   int64_t time_now)
{
    MyQueryState q(u_id, sid, service_name);
    q.is_forwarded_query = false;
    q.query_buffer = ByteBuffer(view_bytes(b.begin(), b.end()));
    q.query_status = QueryStatus::Prepared;
    q.received_responses.resize(n_commands);
    q.time_created = time_now;
    q.time_timeout = q.time_created + timeout;
    return q;
}

// Vse potrebne bylo poslano, vse potrebne do callbacku bylo dano.
bool can_query_be_erased(MyQueryState const &q);

#endif // CM_DB_QUERY_STATE_HH_
