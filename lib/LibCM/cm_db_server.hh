#ifndef CM_DB_SERVER_HH_
#define CM_DB_SERVER_HH_

#include "weak_string.hh"
#include "cm_db_types.hh"

#include <map>

namespace CmDm
{

class Server
{
public:
    virtual ~Server() {}
    
    // Add servive to currently accepted services.
    //
    // forward = yes: Incoming queries should be received in raw
    // form. Used by router.
    //
    // forward = no: Incomping queries shall be parsed.
    //
    virtual void accept_service(WeakString s, DbServiceParams forward) = 0;

    // Clear currently accepted services. Then call `accept_service`
    // for each given service.
    virtual void set_accepted_services(std::map<std::string, DbServiceParams> const &services) = 0;

    // Send response to a query in a raw form. You may call
    // `add_forwarded_response` multiple times, until you pass a
    // message whose kind is Completed. If you use
    // `add_forwarded_response`, you must not use `send_response` on
    // this query.
    virtual void add_forwarded_response(DbMessage msg) = 0;

    // Send rows to a single command of a query. You may call
    // `send_response` multiple times, until the query is completed.
    // If you respond using a `send_response`, you must not use
    // `add_forwarded_response` on this query.
    virtual void send_response(CmDbResponse response) = 0;
};

class Client
{
public:
    virtual ~Client() {}

    // Send query to a given service. If the service is not available,
    // each command of the query ends with an error.
    virtual int64_t send_query(WeakString service, CmDbQuery q) = 0;

    // Send query to a given service. If the service is not available,
    // return Nothing(), and no message is sent.
    virtual Maybe<int64_t> send_query_maybe(WeakString title, CmDbQuery q) = 0;

    // Send query in a raw form to a given service. If the service is
    // not available, return Nothing().
    virtual Maybe<int64_t> begin_forwarded_query(WeakString service);

    // Add data to a query which was previously started by
    // `begin_forwarded_query`. You may call it multiple times until
    // you pass a message with Complete flag set.
    virtual void add_to_forwarded_query(DbMessage msg);
};

}

#endif // CM_DB_SERVER_HH_
