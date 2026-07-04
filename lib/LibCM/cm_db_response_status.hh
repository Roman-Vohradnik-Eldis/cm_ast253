#ifndef CM_DB_RESPONSE_STATUS_HH_
#define CM_DB_RESPONSE_STATUS_HH_

#include "cm_db_query_common.hh"
#include "cm_string_buffer.hh"
#include "compress_base.hh"
#include "compress_none.hh"
#include "compress_zstd.hh"

// Ja odpovidam na partneruv dotaz
struct MyResponseState
{  
    MyResponseState(QueryId u_id,
                    PartnerQueryId p_id,
                    ServiceId serv_id,
                    DbService service_name,
                    bool is_forwarded_query,
                    bool compress)
        : user_query_id(u_id),
          partner_query_id(p_id),
          service_id(serv_id),
          service_name(service_name),
          is_forwarded_query(is_forwarded_query)
    {
    }
    // KONFIGURACE
    ////////////////////////////////////////
    
    QueryId user_query_id;
    PartnerQueryId partner_query_id;
    ServiceId service_id;
    DbService service_name;
    bool is_forwarded_query;
    // bool use_compression;

    // PRIJIMANI DOTAZU
    ////////////////////////////////////////
    
    // Query nam chodi od partnera po kouskach
    std::vector<uint8_t> query_buffer;

    // Az dostaneme oznameni, ze jsme dotaz prijali cely, tak sem dame true.
    bool query_was_completed = false;

    // Pokud je is_forwarded_query == false, tak az nam prijde cela,
    // tak ji navic naparsujeme a dame sem.
    Maybe<CmDbQuery> query;

    // Pak zde dame true, kdyz:
    // pro is_forwarded_query == false: Az si ji uzivatel naparsovanou vyzvedne.
    // pro is_forwarded_query == true: Az si uzivatel vyzvedne cely query_buffer && query_was_completed.
    bool query_was_read = false;

    // Jestli partner poslal cancel. Pokud ano, uz nic nebudeme chtit
    // ohledne tohoto dotazu odesilat. Pouze tuto informaci predame do
    // callbacku.
    bool query_was_cancelled = false;
    std::string cancel_message;

    // ODESILANI ODPOVEDI
    ////////////////////////////////////////

    bool first_response_was_sent = false;
    size_t rows_added = 0;

    // Tohle se pouziva jen pri is_forwarded_query == false.  Uzivatel
    // muze plnit `responses`. Vzdy se vezme cast a serializuje se do
    // response buffer.  Az uzivatel oznami, ze uz nic nebude
    // pridavat, tak nastavime all_responses_prepared na true.
    std::vector<MyReceivedResponse> responses;

    // Sem pridavame zpravu, ktera se ma odesilat.  Postupne odtud
    // ukrajujeme a posilame to.  Pokud nemame forwarded query, je to
    // inicializovane az po te, co obdrzime cely dotaz a zjistime,
    // jestli mame pouzit kompresi, nebo ne.
    ByteBuffer response_buffer;

    // Pokud tohle je true && vsechno jsme odelali, tak muzeme cely
    // MyResponseState zahodit.
    bool all_responses_prepared = false;

    // Uz nechceme posilat odpovedi, chceme poslat error.
    bool responses_error = false;
    std::string error_msg;
};

#endif // CM_DB_RESPONSE_STATUS_HH_
