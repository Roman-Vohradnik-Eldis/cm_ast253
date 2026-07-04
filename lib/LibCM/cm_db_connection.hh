#ifndef CM_DB_CONNECTION_HH_
#define CM_DB_CONNECTION_HH_

#include "cm_db_types.hh"
#include <map>

class CmDbConnection
{
public:
    virtual ~CmDbConnection() {}

    // Ostatnim zarizenim se posle informace, ze poskytujeme sluzbu
    // zadaneho jmena.  Moznost forward == true vyuziva router:
    // prichozi dotazy budou chodit nenparsovane.
    virtual void DbAcceptService(DbService service_name, DbServiceParams = DbServiceParams()) = 0;
    virtual void DbSetAcceptedServices(std::map<DbService, DbServiceParams> const &) = 0;

    // Posle dotaz.  Pokud nikdo neposkytuje zadane `service_name`,
    // vrati to Nothing().  Pokud ano, tak se mu dotaz posle. Vrati to
    // id dotazu. Odpoved prijde v CmChanges s timto id.
    virtual Maybe<QueryId> DbSendQueryMaybe(DbService service_name, CmDbQuery const &query) = 0;

    // Posle dotaz.  Pokud nikdo neposkytuje zadane `service_name`,
    // v pristi CmChanges prijde error, ze to nejde poslat.
    virtual QueryId DbSendQuery(DbService service_name, CmDbQuery const &query) = 0;

    // Predpoklada se, ze v CmChanges prisel dotaz (s nejakym id), a
    // my na toto id ted odpovidame, jinak to vypise chybu a neudela
    // to nic. Na jeden dotaz muzeme odpovidat opakovane, dokud na
    // vsechny prikazy dotazu neodpovime CM_DB_DONE, cimz se bere
    // odpoved jako kompletni.
    virtual void DbSendResponse(CmDbResponse response) = 0;

    // Jako DbSendResponse, ale posilame jiz serializovanou zpravu. Tohle
    // pouziva router.
    virtual void DbForwardResponse(DbMessage msg) = 0;
    
    // Zacne posilat zpravu partnerovi Jako DbSendQuery, ale posilame
    // jiz serializovanou zpravu. Tohle pouziva router.
    virtual Maybe<QueryId> DbBeginForwardedQuery(DbService service) = 0;
    virtual void DbAddToForwardedQuery(DbMessage msg) = 0;

    // Tohle muze zavolat uzivatel kdykoliv po polozeni dotazu.  Do
    // CmChanges prijde informace, ze vsechny prikazu tohoto dotazu,
    // na ktere jsme jeste nedostali odpoved, skoncily errorem.
    // Partnerovi se posle informace, ze se tento dotaz rusi.
    virtual void DbCancelQuery(QueryId query_id, WeakString reason, bool generate_callback) = 0;
};

#endif // CM_DB_CONNECTION_HH_
