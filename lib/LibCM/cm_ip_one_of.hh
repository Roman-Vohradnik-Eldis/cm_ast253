#ifndef CM_IP_ONE_OF_HH_
#define CM_IP_ONE_OF_HH_

#include "cm_debug.hh"
#include "cm_ip.hh"

class CmIpOneOf : public CmIPBase
{
    std::vector<std::unique_ptr<CmIPBase>> candidates;
    std::unique_ptr<CmIPBase> active;
    std::set<unsigned> active_partners;
    std::vector<std::string> ip_descriptors;
    cm::function<void()> callback;
    std::string debug_name;
    CmIPParams default_params;

public:
    CmIpOneOf(std::vector<std::string> const &ip_descriptors,
              cm::function<void()> callback,
              std::string debug_name,
              CmIPParams default_params)
        : ip_descriptors(ip_descriptors),
          callback(callback),
          debug_name(debug_name),
          default_params(default_params)
    {
        fill_candidates();
    }

    void shutdown()
    {
        for (auto &p : candidates)
            p->shutdown();
        if (active)
            active->shutdown();
    }

    Maybe<CmIpEvent> get_new_event()
    {
        if (!active && !active_partners.empty())
        {
            unsigned id = *active_partners.begin();
            active_partners.erase(active_partners.begin());
            return CmIpEvent{
                id,
                false, // connected
                true, // disconnected
                "",
                {}
            };
        }
        
        if (active)
        {
            Maybe<CmIpEvent> e = active->get_new_event();
            process_event(e);
            return e;
        }
        else
        {
            assert(active_partners.empty());
            for (auto &p : candidates)
            {
                Maybe<CmIpEvent> e = p->get_new_event();
                if (e)
                {
                    assert(e->just_connected = true);
                    if (!e->just_disconnected)
                    {
                        active = std::move(p);
                        candidates.clear(); // stop connections
                        process_event(e);
                        return e;
                    }
                }
            }
            return Nothing();
        }
    }

    void send(char const *buffer, int size, unsigned conn_id)
    {
        if (!active)
            cms_ns2_if_print("cmip", debug_name, 1, "Cannot send, partner %u not connected", conn_id);
        else
            active->send(buffer, size, conn_id);
    }

    void disconnect(unsigned conn_id)
    {
        if (!active)
            cms_ns2_if_print("cmip", debug_name, 1, "Cannot disconnect, partner %u not connected", conn_id);
        else
            active->disconnect(conn_id);        
    }

    bool is_ready_to_send(unsigned conn_id)
    {
        if (!active)
        {
            cms_ns2_if_print("cmip", debug_name, 1, "Cannot tell if is ready to send, partner %u not connected", conn_id);
            return false;
        }
        else
        {
            return active->is_ready_to_send(conn_id);
        }
    }

private:
    void process_event(Maybe<CmIpEvent> e)
    {
        if (e)
        {
            assert(active);
            assert(candidates.empty());
            if (e->just_connected)
            {
                assert(!contains(active_partners, e->connection_id));
                active_partners.insert(e->connection_id);
            }
            if (e->just_disconnected)
            {
                assert(contains(active_partners, e->connection_id));
                active_partners.erase(e->connection_id);
            }
            if (active_partners.empty())
            {
                active = nullptr;
                fill_candidates();
            }
        }
    }

    void fill_candidates()
    {
        assert(!active);
        assert(candidates.empty());

        // std::cout << "Fill candidates " << cm_concat(ip_descriptors.begin(), ip_descriptors.end(), "+") << std::endl;
        for (std::string const &s : ip_descriptors)
        {
            try
            {
                std::unique_ptr<CmIPBase> p(create_connection(s, callback, debug_name, default_params));
                candidates.push_back(std::move(p));
            }
            catch (std::exception &e)
            {
                cms_ns2_if_print("cmip", debug_name, 1, "Failed to create connection '%s': %s", s.c_str(), e.what());                
            }
        }
    }
};

#endif // CM_IP_ONE_OF_HH_
