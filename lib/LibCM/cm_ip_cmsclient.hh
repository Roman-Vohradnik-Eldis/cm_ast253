#ifndef CM_IP_CMSCLIENT_HH_
#define CM_IP_CMSCLIENT_HH_

#include "cm_ip.hh"
#include "cm_ip_eventstate.hh"
#include "cm_header_internal.hh"

class CmIP_CMSClient : public CmIPBase
{
    cm::function<void()> notify_cb;
    cm::scoped_ptr<CMSClient> cms;

    // Inkrementujeme s kazdym znovupripojenim
    unsigned the_conn_id;

    std::string m_name;

    atomic_bool stop;
    EventsState events;
    Thread recv_thread;

public:
    CmIP_CMSClient(cm::function<void()> cb, CmIpAddress addr, std::string name, CMS_TCP_MODE mode)
	: notify_cb(cb),
	  cms(new CMSClient(CMS_FLAG_BOTH, addr.addr, addr.port, addr.ping, mode)),
          the_conn_id(55),
	  m_name(name),
          events(name),
	  recv_thread(cm::bind_front(&CmIP_CMSClient::recv_main, this), "tcpcli")
    {
	cms_ns2_if_print("cmsclient", m_name, 3, "CMSClient created, connecting to %s %u", addr.addr.c_str(), addr.port);
    }

    ~CmIP_CMSClient()
    {
        shutdown();
    }

    bool is_ready_to_send(unsigned conn_id)
    {
        // Kdyby CMS umela rict, jestli ma misto v bufferu, tak by to bylo super.
        return true;
    }

    void shutdown()
    {
        cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient shutdown");
        stop = true;
        CMSShutdown(*cms);
    }

    void send(char const *buffer, int size, unsigned conn_id)
    {
	if (stop)
	    return;
	if (events.is_connected(conn_id))
	{
	    cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient send to %u (%d bytes): %s",
                             conn_id, size, replace_nonprintable(WeakString(buffer, buffer + size)).c_str());
	    cms->Send(buffer, size);
	    cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient send to %u done", conn_id);
	}
	else
	{
	    cms_ns2_if_print("cmsclient", m_name, 2, "CMSClient cannot send, id %u is not connected", conn_id);
	}
    }    
    void disconnect(unsigned conn_id)
    {
	if (stop)
	    return;
	if (events.is_connected(conn_id))
	{
	    cms_ns2_if_print("cmsclient", m_name, 3, "CMSClient disconnecting %u", conn_id);
	    cms->Disconnect();
	}
        events.erase(conn_id);
    }

    Maybe<CmIpEvent> get_new_event()
    {
	return events.get_new_event();
    }    
    
private:
    std::string create_client_address(unsigned connection_id, CMSClient &client)
    {
	char buffer[128]; 
	snprintf(buffer, sizeof buffer, "id=%u local=%s:0x%x dest=%s:0x%x",
		 connection_id,
		 CMSClient_GetLocalAddr(client).c_str(),
		 CMSClient_GetLocalPort(client),
		 CMSClient_GetRemoteAddr(client).c_str(),
		 CMSClient_GetRemotePort(client));
	return buffer;
    }

    
    void recv_main()
    {
	CMSClientInit(*cms);
	char buffer[MSGMAX];
	bool connected = false;
        bool do_notify = false;
	cms_ns2_if_print("cmsclient", m_name, 4, "Recv Started");
	while (!stop)
	{
	    if (cms->IsConnected())
	    {
                if (!connected)
		{
		    connected = true;
		    ++the_conn_id;
		    events.connect(the_conn_id, create_client_address(the_conn_id, *cms));
                }
                else
                {
                    cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient waiting on recv");
                    int length = cms->Recv(buffer, MSGMAX);
                    if (length > 0)
                    {
                        cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient received (%d bytes):%s", length,
                                         replace_nonprintable(WeakString(buffer, buffer + length)).c_str());
                        events.add_message(the_conn_id, buffer, length);
                    }
                }
                do_notify = true;
	    }
	    else
	    {
		if (connected)
		{
                    do_notify = true;
		    connected = false;
		    events.disconnect(the_conn_id);
		}
		else
		{
		    // TODO: Connect blokuje? nevim.
		    cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient calling Connect");
		    cms->Connect();
                    if (!cms->IsConnected())
                        usleep(500000);
		    cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient: connected=%d", cms->IsConnected());
		}
	    }
            if (do_notify)
            {
                do_notify = false;
                cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient notifying");
                notify_cb();
                cms_ns2_if_print("cmsclient", m_name, 4, "CMSClient notified");
            }
	}
    }
};

#endif // CM_IP_CMSCLIENT_HH_
