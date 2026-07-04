#ifndef CM_IP_UDP_PARTNER_LIST_HH_
#define CM_IP_UDP_PARTNER_LIST_HH_

#include "cm_ip_udp_message.hh"
#include "cm_ip_udp_partner.hh"

struct AcceptedSession
{
    std::string hash;
    SessionId session_id;
    int64_t his_initial_msg_id;
    int64_t my_initial_msg_id;
    int64_t time_created;
};

// 1. Jednak si drzime potencialni partnery (ty, kterym jsme poslali SESSION_ID)
//
// 2. Drzime si aktivni partnery (kteri nam uz poslali prvni zpravu).
//
class PartnerList
{
    std::list<Partner> partners;
    unsigned conn_counter;

    // Tohle je pro UDP server
    SessionIdCounter session_id_counter;
    std::vector<AcceptedSession> accepted_sessions;

public:
    typedef std::list<Partner>::iterator iterator;
    typedef std::list<Partner>::const_iterator const_iterator;

    PartnerList()
        : conn_counter(1)
    {}
    
    iterator begin() { return partners.begin(); }
    iterator end() { return partners.end(); }
    const_iterator begin() const { return partners.begin(); }
    const_iterator end() const { return partners.end(); }

    // Tohle poziva server, kdyz mu prijde HASH od clienta.  Pokud by
    // client poslal jeden HASH vicekrat, musime mu poslat vzdy stejne
    // SESSION_ID.
    //
    AcceptedSession const &get_or_create_session_id(WeakString hash, int64_t his_initial_msg_id);
    
    // Kdyz nam prijde HASH a my odesleme SESSION_ID, tak se jeste
    // partner nevytvori. Vytvori se az tehdy, kdyz Client posle prvni
    // zpravu. V te chvili si timhle zkontruljeme, jestli jsme toto
    // SESSTIO_ID posilali.
    //
    AcceptedSession const *find_session_id(SessionId session_id);
    
    bool empty() const
    {
	return partners.empty();
    }

    size_t size() const
    {
	return partners.size();
    }

    Partner &add_partner(uint32_t address, uint16_t port, CmIPParams params, SessionId session_id);
    
    Partner *find_by_cmip_id(unsigned cmip_id);
    
    Partner *find_by_session_id(SessionId session_id);
    
    void erase_partner_by_cmip_id(unsigned cmip_id);

private:
    SessionId alloc_fresh_session_id();
    void erase_old_session_ids();
};

#endif // CM_IP_UDP_PARTNER_LIST_HH_
