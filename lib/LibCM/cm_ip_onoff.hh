#ifndef CM_IP_ONOFF_HH_
#define CM_IP_ONOFF_HH_

#include "cm_debug.hh"
#include "cm_ip.hh"
#include "cm_utils.hh"

class CmIpOnOff : public CmIPBase
{
    // enabled -> CmIpBase
    // not enabled -> nullptr
    std::unique_ptr<CmIPBase> device;
    bool need_erase_partners = false;
    std::set<unsigned> connected_partners;

    // Partneri z predchoziho spojeni. Nez zacneme pouzivat nove
    // spojeni, tak musime vsechny predat jako disconnected, jinak by
    // se nam mohly pomichat IDčka.
    CmIpDescriptorSub ip_descriptor;
    cm::function<void()> callback;
    std::string debug_name;
    CmIPParams default_params;

public:
    CmIpOnOff(CmIpDescriptorSub ip_descriptor,
              cm::function<void()> callback,
              std::string debug_name,
              CmIPParams default_params)
        : ip_descriptor(ip_descriptor),
          callback(callback),
          debug_name(debug_name),
          default_params(default_params)
    {}

    void set_enabled(bool b)
    {
        if (b && !device)
        {
            device.reset(create_connection(ip_descriptor, callback, debug_name, default_params));
        }
        else if (!b && device)
        {
            need_erase_partners = true;
            device = nullptr; // shutdown connections
            if (!connected_partners.empty())
                callback();
        }
    }

    bool is_ready_to_send(unsigned conn_id)
    {
        if (!device)
        {
            cms_ns2_if_print("cmip", debug_name, 1, "Cannot tell if ready to send, partner %u not connected", conn_id);
            return false;
        }
        return device->is_ready_to_send(conn_id);
    }

    void send(char const *buffer, int size, unsigned conn_id)
    {
        if (!device)
        {
            cms_ns2_if_print("cmip", debug_name, 1, "Cannot send, partner %u not connected", conn_id);
            return;
        }
        device->send(buffer, size, conn_id);
    }

    void disconnect(unsigned conn_id)
    {
        if (!device)
        {
            cms_ns2_if_print("cmip", debug_name, 1, "Cannot disconnect partner %u, not connected", conn_id);
            return;
        }
        device->disconnect(conn_id);
    }

    void shutdown()
    {
        if (device)
        {
            device->shutdown();
        }
    }

    Maybe<CmIpEvent> get_new_event()
    {
        if (need_erase_partners)
        {
            if (connected_partners.empty())
            {
                need_erase_partners = false;
            }
            else
            {
                auto id = *connected_partners.begin();
                connected_partners.erase(connected_partners.begin());
                return CmIpEvent{
                    id,
                    false, // just_connected
                    true, // just_disconnected
                    "",
                    {}
                };
            }
        }
        if (!device)
        {
            return Nothing();
        }
        else
        {
            Maybe<CmIpEvent> e = device->get_new_event();
            if (e)
            {
                if (e->just_connected)
                {
                    assert(!contains(connected_partners, e->connection_id));
                    connected_partners.insert(e->connection_id);
                }
                if (e->just_disconnected)
                {
                    assert(contains(connected_partners, e->connection_id));
                    connected_partners.erase(e->connection_id);
                }
            }
            return e;
        }
    }
};

#endif // CM_IP_ONOFF_HH_
