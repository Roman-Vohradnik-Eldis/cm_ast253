#ifndef CM_IP_CMSSSEERVER_H_
#define CM_IP_CMSSSEERVER_H_

#include "cm_ip.hh"
#include "cm_ip_eventstate.hh"
#include "cm_header_internal.hh"
#include "cm_scheduler.hh"

class CmIP_CMSServer2 : public CmIPBase
{
    cm::function<void()> notify_cb;
    cm::scoped_ptr<CMSServer2> cms;

    CMSMutex mutex;
    atomic_bool stop;
    
    EventsState events;

    std::string m_name;

    cm::scoped_ptr<PollCond> notify_cond;
    Thread recv_thread;
    // ThreadWorker<Nothing> notify_worker;

    static gboolean onServerConnect(cms_client_t *client, void * data);
    static gboolean onServerDisconnect(cms_client_t *client, void * data);

public:
    CmIP_CMSServer2(cm::function<void()> cb, CmIpAddress addr, std::string name, CMS_TCP_MODE mode)
	: notify_cb(cb),
	  cms(new CMSServer2(CMS_FLAG_BOTH,
                             addr.addr,
                             addr.port,
                             addr.ping,
                             mode,
                             CMSServer2CB(CmIP_CMSServer2::onServerConnect, this),
                             CMSServer2CB(CmIP_CMSServer2::onServerDisconnect, this)
                  )),
          events(name),
	  m_name(name),
          notify_cond(new PollCond(cm::bind_front(&CmIP_CMSServer2::notify, this), "/CMSNotif")),
	  recv_thread(cm::bind_front(&CmIP_CMSServer2::recv_main, this), "tcpsrv")
	  // notify_worker(cm::bind_front(&CmIP_CMSServer2::notify, this), "tcpntf")
    {
	cms_ns2_if_print("cmip", m_name, 3, "CMSServer created, listening on %s %u", addr.addr.c_str(), addr.port);
	cms->Init();
    }

    ~CmIP_CMSServer2()
    {
        shutdown();
    }

    void shutdown()
    {
        cms_ns2_if_print("cmip", m_name, 4, "CMSServer shutdown");
        stop = true;
	CMSShutdown(*cms);        
    }

    bool is_ready_to_send(unsigned conn_id)
    {
        // Kdyby CMS umela rict, jestli ma misto v bufferu, tak by to bylo super.
        return true;
    }

    void send(char const *buffer, int size, unsigned conn_id)
    {
        if (stop)
            return;
	if (events.is_connected(conn_id))
	{
	    cms_ns2_if_print("cmip", m_name, 4, "CMSServer send to %u: (%d bytes): %s", conn_id, size, replace_nonprintable(WeakString(buffer, buffer + size)).c_str());
	    cms->Send(buffer, size, conn_id);
	    cms_ns2_if_print("cmip", m_name, 5, "CMSServer send to %u done", conn_id);
	}
	else
	{
	    cms_ns2_if_print("cmip", m_name, 2, "CMSServer cannot send, id %u is not connected", conn_id);
	}
    }    

    void disconnect(unsigned conn_id)
    {
        if (stop)
            return;
	if (events.is_connected(conn_id))
	{
	    cms_ns2_if_print("cmip", m_name, 3, "CMSServer disconnecting %u", conn_id);
	    cms->Disconnect(conn_id);
        }
        events.erase(conn_id);
    }

    Maybe<CmIpEvent> get_new_event()
    {
	return events.get_new_event();
    }

private:
    void recv_main()
    {
	add_debug_thread("srv-recv");
	char buffer[MSGMAX];
	cms_ns2_if_print("cmip", m_name, 4, "Recv Started");
	for (;;)
	{
            if (stop)
                break;

	    if (!cms->IsConnected())
	    {
		usleep(50000);
		continue;
	    }
	    unsigned connection_id;
	    cms_ns2_if_print("cmip", m_name, 4, "CMSServer waiting on recv");
	    int length = CMSServer2Recv(*cms, buffer, MSGMAX, &connection_id);
	    if (length > 0)
	    {
		cms_ns2_if_print("cmip", m_name, 4, "Recv OK (%d bytes): %s", length, replace_nonprintable(WeakString(buffer, buffer + length)).c_str());
		events.add_message(connection_id, buffer, length);
                notify_cond->Wake();
		// notify_worker.push(Nothing());
	    }
	}
    }

    void notify()
    {
	notify_cb();
    }

    std::string create_client_address(unsigned connection_id, cms_client_t *client)
    {
	char buffer[128]; 
	snprintf(buffer, sizeof buffer, "id=%u local=%s:0x%x dest=%s:0x%x",
		 connection_id,
		 CMS_client_get_local_addr(client).c_str(),
		 CMS_client_get_local_port(client),
		 CMS_client_get_remote_addr(client).c_str(),
		 CMS_client_get_remote_port(client));
	return buffer;
    }

    bool on_connect(cms_client_t *client)
    {
	cms_ns2_if_print("cmip", m_name, 4, "CMSServer on_connect");
	unsigned conn_id = cms_client_conenction_id(client);
	events.connect(conn_id, create_client_address(conn_id, client));
        notify_cond->Wake();
	// notify_worker.push(Nothing());
	return true;
    }

    bool on_disconnect(cms_client_t *client)
    {
	cms_ns2_if_print("cmip", m_name, 4, "CMSServer on_disconnect");
	unsigned conn_id = cms_client_conenction_id(client);
	events.disconnect(conn_id);
        notify_cond->Wake();
	// notify_worker.push(Nothing());
	return true;
    }
};

#endif // CM_IP_CMSSSEERVER_H_
