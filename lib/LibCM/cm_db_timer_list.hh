#ifndef CM_DB_TIMER_LIST_HH_
#define CM_DB_TIMER_LIST_HH_

#include "cm_db_query_list.hh"
#include "cm_db_response_list.hh"
#include "cm_db_types.hh"
#include "cm_send_buffer_simple.hh"

/// Tohle je podpora pro specialni dotazy tvaru:
///
/// wait TIME
///
/// kde TIME je cas v mikrosekundach. Odpoved DONE prijde za TIME od
/// odeslani.
///
/// Mozna je to jenom docasne reseni, ze casem to spadne pod neco
/// obecnejsiho.
///
class CmDbTimerList
{
    // Kazde query_id je bud ve waiting nebo v expired
    //
    // V expired muze byt vyjimecne vicekrat (pokud uzivatel zadal
    // spatny dotaz s vice prikazy, coz neni povoleno).
    //
    std::multimap<int64_t, CmDbReceivedResponse> waiting;
    std::vector<CmDbReceivedResponse> expired;

public:
    bool wakeup(int64_t time_now);

    Maybe<int64_t> get_next_wakeup_time();

    std::vector<CmDbReceivedResponse> get_received_responses();

    bool has_query_id(QueryId q);

    void add_query(QueryId id, DbService service, CmDbQuery const &q, int64_t time_now);

    void cancel_query(QueryId, WeakString reason, bool gen_callback);

private:
    void add_error_query();
};

#endif // CM_DB_TIMER_LIST_HH_
