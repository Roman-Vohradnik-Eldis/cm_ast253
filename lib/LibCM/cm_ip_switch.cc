#include "cm_ip_switch.hh"
#include "cm_debug.hh"
#include "cm_utils.hh"

CmIpSwitch::CmIpSwitch(std::vector<std::string> const &addr_list,
                       cm::function<void()> cb,
                       std::string debug_name,
                       CmIPParams default_params)
    : debug_name(debug_name)
{
    for (std::string const &addr : addr_list)
    {
        CmIpDevice d;
        d.name = format("%s/%zu", debug_name.c_str(), devices.size());
        d.ip.reset(create_connection(addr, cb, debug_name, default_params));
        devices.push_back(std::move(d));
    }
}

template <typename Map>
unsigned get_unused_key(Map const &m)
{
    for (typename Map::key_type i = 100; ; ++i)
        if (!contains(m, i))
            return i;
}

template <typename Map, typename T>
typename Map::key_type const *map_find_value(Map const &m, T const &val)
{
    for (typename Map::const_iterator it = m.begin(); it != m.end(); ++it)
        if (it->second == val)
            return &it->first;
    return 0;
}

unsigned CmIpSwitch::allocate_user_id(CmIpDevice &d, unsigned dev_conn_id)
{
    unsigned user_conn_id = get_unused_key(user_connections);
    user_connections.insert({user_conn_id, &d});
    d.dev_conn_id_to_user_conn_id.insert({dev_conn_id, user_conn_id});
    return user_conn_id;
}

// Vrati true, pokud Event mame dat uzivateli
bool CmIpSwitch::handle_and_adjust_event(CmIpDevice &d, CmIpEvent &ev)
{
    unsigned dev_conn_id = ev.connection_id;
    // cms_ns2_if_print("cmip", d.name, 1, "handle event dev_conn_id=%u, just_connected=%d, just_disconnected=%d",
    //                  dev_conn_id, ev.just_connected, ev.just_disconnected);
    // Create new connection if needed
    if (ev.just_connected)
    {
        if (!contains(d.dev_conn_id_to_user_conn_id, dev_conn_id))
        {
            unsigned u = allocate_user_id(d, dev_conn_id);
            cms_ns2_if_print("cmip", d.name, 1, "CmIpSwitch new connection: user_id=%u -> dev_conn_id=%u", u, dev_conn_id);
        }
        else
        {
            cms_ns2_if_print("cmip", d.name, 1, "Interal error: just_connected, but already known dev_conn_id=%u", dev_conn_id);
        }
    }
    // Replace connection id
    unsigned const *user_conn_id = map_find(d.dev_conn_id_to_user_conn_id, dev_conn_id);
    if (!user_conn_id)
    {
        // Dokud neprijde od tohoto connection_id just_connected, budeme vsechny eventy zahazovat
        cms_ns2_if_print("cmip", d.name, 1, "Interal error: Event from unknown dev_conn_id %u", dev_conn_id);
        return false;
    }
    ev.connection_id = *user_conn_id;
    
    // Erase connection if needed
    if (ev.just_disconnected)
    {
        d.dev_conn_id_to_user_conn_id.erase(dev_conn_id);
        user_connections.erase(*user_conn_id);
    }
    return true;
}

Maybe<CmIpEvent> CmIpSwitch::get_new_event()
{
    for (CmIpDevice &d : devices)
    {
        if (Maybe<CmIpEvent> e = d.ip->get_new_event())
        {
            if (handle_and_adjust_event(d, *e))
                return e;
        }
    }
    return Nothing();
}

Maybe<std::pair<CmIpDevice *, unsigned>> CmIpSwitch::find_connection_id(unsigned user_conn_id)
{
    CmIpDevice **d = map_find(user_connections, user_conn_id);
    if (!d)
    {
        cms_ns2_if_print("cmip", debug_name, 1, "Connection id %u does not exist", user_conn_id);
        return Nothing();
    }
    unsigned const *dev_conn_id = map_find_value((*d)->dev_conn_id_to_user_conn_id, user_conn_id);
    if (!dev_conn_id)
    {
        cms_ns2_if_print("cmip", debug_name, 1, "Internal error: Connection id %u does not exist", user_conn_id);
        return Nothing();
    }
    std::pair<CmIpDevice *, unsigned> p(*d, *dev_conn_id);
    return p;
}

void CmIpSwitch::set_params(unsigned user_conn_id, CmIPParams params)
{
    if (auto dev_conn = find_connection_id(user_conn_id))
        dev_conn->first->ip->set_params(dev_conn->second, params);
}

void CmIpSwitch::send(char const *buffer, int size, unsigned user_conn_id)
{
    if (auto dev_conn = find_connection_id(user_conn_id))
        dev_conn->first->ip->send(buffer, size, dev_conn->second);
}

bool CmIpSwitch::is_ready_to_send(unsigned user_conn_id)
{
    if (auto dev_conn = find_connection_id(user_conn_id))
        dev_conn->first->ip->is_ready_to_send(dev_conn->second);
    return true;
}

void CmIpSwitch::disconnect(unsigned user_conn_id)
{
    if (auto dev_conn = find_connection_id(user_conn_id))
        dev_conn->first->ip->disconnect(dev_conn->second);
}

void CmIpSwitch::shutdown()
{
    for (auto &p : user_connections)
        p.second->ip->shutdown();
}

