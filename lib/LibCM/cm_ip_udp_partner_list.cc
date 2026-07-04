#include "cm_ip_udp_partner_list.hh"
#include "cm_ip_udp_message.hh"

SessionId PartnerList::alloc_fresh_session_id()
{
    return session_id_counter.get_fresh([this](SessionId id){return !find_session_id(id);});
}

void PartnerList::erase_old_session_ids()
{
    if (accepted_sessions.size() >= 1000)
    {
        std::set<SessionId> used_ids;
        for (Partner const &p : partners)
            used_ids.insert(p.session_id);
        auto p = [&](AcceptedSession s){return !contains(used_ids, s.session_id);};
        auto it = std::remove_if(accepted_sessions.begin(), accepted_sessions.end(), p);
        accepted_sessions.erase(it, accepted_sessions.end());
    }
}

AcceptedSession const *PartnerList::find_session_id(SessionId id)
{
    for (AcceptedSession &s : accepted_sessions)
    {
        if (s.session_id == id)
            return &s;
    }
    return nullptr;
}

AcceptedSession const &PartnerList::get_or_create_session_id(WeakString hash, int64_t his_initial_msg_id)
{
    CM_for (AcceptedSession &s, accepted_sessions)
    {
        if (s.hash == hash)
            return s;
    }
    erase_old_session_ids();
    AcceptedSession as;
    as.hash = hash;
    as.session_id = alloc_fresh_session_id();
    as.time_created = get_current_time_usec();
    as.his_initial_msg_id = his_initial_msg_id;
    as.my_initial_msg_id = cm_get_random_value<uint64_t>() % msg_id_end;
    accepted_sessions.push_back(as);
    return accepted_sessions.back();
}

Partner &PartnerList::add_partner(uint32_t address, uint16_t port, CmIPParams params, SessionId session_id)
{
    partners.push_back(Partner());
    
    Partner &p = partners.back();
    p.cmip_id = conn_counter++;
    p.address = address;
    p.port = port;
    p.udp_params = params;
    p.session_id = session_id;
    return partners.back();
}

Partner *PartnerList::find_by_cmip_id(unsigned cmip_id)
{
    CM_for (Partner &p, partners)
        if (p.cmip_id == cmip_id)
            return &p;
    return 0;
}

Partner *PartnerList::find_by_session_id(SessionId session_id)
{
    CM_for (Partner &p, partners)
        if (p.session_id == session_id)
            return &p;
    return 0;
}

void PartnerList::erase_partner_by_cmip_id(unsigned cmip_id)
{
    for (std::list<Partner>::iterator it = partners.begin(); it != partners.end(); ++it)
    {
        if (it->cmip_id == cmip_id)
        {
            partners.erase(it);
            break;
        }
    }
}    
