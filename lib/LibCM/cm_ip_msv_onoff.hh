#ifndef CM_IP_MSV_ONOFF_HH_
#define CM_IP_MSV_ONOFF_HH_

#include "cm_ip_onoff.hh"

struct MasterSlaveItem
{
    std::string group;
    std::string master_station;
    std::vector<std::string> alive_stations;
};

// Zprava je tvaru:
// SSR:SSR1:SSR1+SSR2 TIME:TIME1:TIME1+TIME2 CMS:LCMS2:LCMS1+LCMS2+RMM2
//
inline std::map<std::string, MasterSlaveItem> parse_master_slave_message(std::string const &msg)
{
    std::map<std::string, MasterSlaveItem> result;
    std::vector<std::string> items = cm_split_by_delim(msg, ' ');
    for (std::string const &item : items)
    {
        std::vector<std::string> stations = cm_split_by_delim(item, ':');
        if (stations.size() < 3)
        {
            cms_ns_if_print("msv", 1, "Received invalid string: `%s'\n", msg.c_str());
            continue;
        }
        std::string group = stations[0];
        std::string master = stations[1];
        std::vector<std::string> alive = cm_split_by_delim(stations[2], ':');
        if (contains(result, group))
        {
            cms_ns_if_print("msv", 1, "Received duplicate group `%s` in: `%s'\n", group.c_str(), msg.c_str());
            continue;
        }
        result.insert({group,
                       {
                           group,
                           master,
                           alive,
                       }});
    }
    return result;
}

class CmIPMasterSlaveOnOff : public CmIPBase
{
    std::unique_ptr<CmIpOnOff> device;
    std::unique_ptr<CmIPBase> master_slave_connection;
    std::string debug_name;
    std::string group;
    std::string my_name;
    
public:
    CmIPMasterSlaveOnOff(CmIpDescriptorSub const &ip_descriptor,
                         cm::function<void()> callback,
                         std::string debug_name,
                         CmIPParams default_params,
                         MasterSlaveParams master_slave_params)
        : device(new CmIpOnOff(ip_descriptor, callback, debug_name, default_params)),
          master_slave_connection(create_connection(master_slave_params.ip_descriptor, callback, debug_name)),
          debug_name(debug_name),
          group(master_slave_params.group),
          my_name(master_slave_params.my_name)
    {
    }
    
    Maybe<CmIpEvent> get_new_event()
    {
        while (Maybe<CmIpEvent> e = master_slave_connection->get_new_event())
        {
            for (std::string const &s : e->messages)
                handle_master_slave_message(s);
        }
        return device->get_new_event();
    }

    bool is_ready_to_send(unsigned conn_id)
    {
        return device->is_ready_to_send(conn_id);
    }

    void send(char const *buffer, int size, unsigned conn_id)
    {
        return device->send(buffer, size, conn_id);
    }

    void disconnect(unsigned conn_id)
    {
        return device->disconnect(conn_id);
    }
    void shutdown()
    {
        device->shutdown();
    }

private:
    void handle_master_slave_message(std::string const &s)
    {
        std::map<std::string, MasterSlaveItem> items = parse_master_slave_message(s);
        if (MasterSlaveItem const *item = map_find(items, group))
        {
            bool is_master = item->master_station == my_name;
            cms_ns2_if_print("cmip", debug_name, 3, "Received MasterSlave message, connection %s",
                            is_master ? "enabled" : "disabled");
            device->set_enabled(is_master);
        }
        else
        {
            cms_ns2_if_print("cmip", debug_name, 2, "Received MasterSlave message, but group '%s' not present: '%s'",
                            group.c_str(), s.c_str());
        }
    }
};

#endif // CM_IP_MSV_ONOFF_HH_
