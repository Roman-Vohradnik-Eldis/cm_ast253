#ifndef CM_IP_SWITCH_HH_
#define CM_IP_SWITCH_HH_

#include "cm_ip.hh"

struct CmIpDevice
{
    std::unique_ptr<CmIPBase> ip;
    std::string name; // for debugging
    std::map<unsigned, unsigned> dev_conn_id_to_user_conn_id;
};

class CmIpSwitch : public CmIPBase
{
    std::string debug_name;
    std::vector<CmIpDevice> devices;

    // Preklad connection id
    std::map<unsigned, CmIpDevice *> user_connections;
    
public:
    CmIpSwitch(std::vector<std::string> const &addr_list,
               cm::function<void()> cb,
               std::string name,
               CmIPParams default_params);

    Maybe<CmIpEvent> get_new_event();
    void send(char const *buffer, int size, unsigned conn_id);
    void disconnect(unsigned conn_id);
    void set_params(unsigned conn_id, CmIPParams params);
    bool is_ready_to_send(unsigned conn_id);
    void shutdown();

private:
    bool handle_and_adjust_event(CmIpDevice &d, CmIpEvent &ev);
    Maybe<std::pair<CmIpDevice *, unsigned>> find_connection_id(unsigned user_conn_id);
    unsigned allocate_user_id(CmIpDevice &d, unsigned dev_conn_id);
};

#endif // CM_IP_SWITCH_HH_
