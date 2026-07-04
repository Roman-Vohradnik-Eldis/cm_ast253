#include <algorithm>
#include <cctype>
#include <memory>
#include <strings.h>
#include "cm_debug.hh"
#include "cm_ip.hh"
#include "cm_ip_msv_onoff.hh"
#include "cm_ip_one_of.hh"
#include "cm_thread.hh"
#include "cm_utils.hh"

#include "cm_ip_cmsclient.hh"
#include "cm_ip_cmsserver.hh"
#include "cm_ip_udp.hh"
#include "cm_ip_switch.hh"

Maybe<int64_t> parse_long_int(std::string str)
{
    if (str.empty())
	return Nothing();
    char *p;
    long l = strtol(str.c_str(), &p, 0);
    if (*p != '\0')
	return Nothing();
    return l;
}

Maybe<uint16_t> parse_uint16_t(std::string str)
{
    Maybe<int64_t> l = parse_long_int(str);
    if (l && *l >= 0 && *l < 0xffff)
	return *l;
    return Nothing();
}

struct CmIP_Null : CmIPBase
{
    void send(char const *buffer, int size, unsigned conn_id)
    {
        assert(false);
        throw std::logic_error("Not connected");
    }

    void disconnect(unsigned conn_id)
    {
        assert(false);
        throw std::logic_error("Not connected");
    }
    Maybe<CmIpEvent> get_new_event()
    {
        return Nothing();
    }

    bool is_ready_to_send(unsigned conn_id)
    {
        assert(false);
        throw std::logic_error("Not connected");        
    }

    void shutdown()
    {
    }

    void set_params(unsigned conn_id, CmIPParams params)
    {
    }
};

CmIpAddress parse_ip(std::vector<std::string> const &fields)
{
    CHECK(fields.size() == 3, "Bad number of fields, expected ADDR:PORT:PING, got %s",
          cm_concat(fields.begin(), fields.end(), ",").c_str());
    CmIpAddress a;
    a.addr = fields[0];
    a.port = parse_uint16_t(fields[1]).get_or_throw("Failed to parse port: " + fields[1]);
    a.ping = parse_long_int(fields[2]).get_or_throw("Failed to parse ping: " + fields[2]);
    return a;
}

struct ClientIp
{
    CmIpAddress listen;
    CmIpAddress connect_to;
};

CmIpAddress default_client_ip()
{
    CmIpAddress client_a;
    client_a.addr = "0.0.0.0"; // Listen on all interfaces
    client_a.port = 0; // Bind to free dynamic UDP port
    client_a.ping = 0; // not used
    return client_a;
}

ClientIp parse_client_ip(std::vector<std::string> const &fields)
{
    if (fields.size() == 3)
    {
        return {default_client_ip(), parse_ip(fields)};
    }
    else if (fields.size() == 4 || fields.size() == 5)
    {
        ClientIp result;
        result.listen.addr = fields[0];
        result.listen.port = parse_uint16_t(fields[1]).get_or_throw("Failed to parse port: " + fields[1]);
        result.listen.ping = 0;
        result.connect_to.addr = fields[2];
        result.connect_to.port = parse_uint16_t(fields[3]).get_or_throw("Failed to parse port: " + fields[3]);
        result.connect_to.ping = 0;
        return result;
    }
    else
    {
        throw std::runtime_error("Bad number of fields, expected SENDTO_ADDR:PORT:PING or LISTEN_ADDR:PORT:SENDTO_ADDR:PORT:PING");
    }
}

bool are_equal_icase(std::string const &a, std::string const &b)
{
    return strcasecmp(a.c_str(), b.c_str()) == 0;
}

CmIPBase *create_connection(std::string ip_descriptor,
                            cm::function<void()> notify_cb,
                            std::string name,
                            CmIPParams default_params)
{
    try
    {
        std::vector<std::string> all_fields = cm_split_by_delim(ip_descriptor, ':');

        CHECK(!all_fields.empty(), "Bad file descriptor");
        std::string type = all_fields[0];
        std::vector<std::string> fields(all_fields.begin() + 1, all_fields.end());

        if (type == "server-udp")
            return new CmIP_UDP(notify_cb, parse_ip(fields), Nothing(), default_params, name);
        if (type == "client-udp")
        {
            auto ip = parse_client_ip(fields);
            return new CmIP_UDP(notify_cb, ip.listen, ip.connect_to, default_params, name);
        }
        if (type == "server-text")
            return new CmIP_CMSServer2(notify_cb, parse_ip(fields), name, CMS_TCP_MODE_TEXT);
        if (type == "server")
            return new CmIP_CMSServer2(notify_cb, parse_ip(fields), name, CMS_TCP_MODE_ELDIS);
        if (type == "client-text")
            return new CmIP_CMSClient(notify_cb, parse_ip(fields), name, CMS_TCP_MODE_TEXT);
        if (type == "client")
            return new CmIP_CMSClient(notify_cb, parse_ip(fields), name, CMS_TCP_MODE_ELDIS);
        if (type == "null")
            return new CmIP_Null();
        throw std::runtime_error("Invalid IP type");
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Failed to create connection '" + ip_descriptor + "': " + e.what());
    }
}

CmIPBase *create_connection(CmIpDescriptorSub ip_descriptor,
                            cm::function<void()> notify_cb,
                            std::string name,
                            CmIPParams default_params)
{
    if (ip_descriptor.ip_descriptors.size() == 1)
    {
        return create_connection(ip_descriptor.ip_descriptors.at(0), notify_cb, name, default_params);
    }
    else if (ip_descriptor.use_single_connection)
    {
        return new CmIpOneOf(ip_descriptor.ip_descriptors, notify_cb, name, default_params);
    }
    else
    {
        return new CmIpSwitch(ip_descriptor.ip_descriptors, notify_cb, name, default_params);
    }
}


CmIPBase *create_connection(CmIpDescriptor ip_descriptor,
                            cm::function<void()> notify_cb,
                            std::string name,
                            CmIPParams default_params)
{
    if (ip_descriptor.only_when_master)
    {
        return new CmIPMasterSlaveOnOff(ip_descriptor.descriptor,
                                        notify_cb,
                                        name,
                                        default_params,
                                        *ip_descriptor.only_when_master);
    }
    else
    {
        return create_connection(ip_descriptor.descriptor, notify_cb, name, default_params);
    }
}

std::string replace_nonprintable(WeakString str)
{
    std::string s = str;
    for (char &c : s)
    {
        if (!isgraph(c) && c != ' ')
            c = '?';
    }
    return s;
}

MasterSlaveParams parse_master_slave_params(std::string const &line)
{
    std::vector<std::string> params = cm_split_trim_by_delim(line, ',');
    CHECK(params.size() == 3, "Bad number of parameters %zu, expected NAME,GROUP,IP", params.size());
    return {
        params.at(2),
        params.at(1),
        params.at(0),
    };
}

// Tedy nevim jednu vec. V konfiguraku se bezne oddeluji polozky
// carkou, ale IP adresy se oddeluji mezerou. Tak jsem to sem dodelal,
// at je to stejny jako v ostatnich Eldis programech.
//
// Navic nektere konfiguraky tam maji i strednik, tak ho podporujeme taky
std::vector<std::string> parse_ip_descriptor_list(std::string const &line)
{
    auto fields = cm_split_trim_by_any_delim(line, ",;");
    return cm_append_map(cm_split_trim_by_spaces, fields);
}
