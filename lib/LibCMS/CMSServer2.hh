#ifndef _CMS_SERVER_2_HH
#define _CMS_SERVER_2_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_server_2.h"

class CMSServer2
{
	cms_server_2_t * server;
	
  public:
	CMSServer2 (
		int flags, 
		std::string addr, 
		std::string port, 
		unsigned ping,
		enum CMS_TCP_MODE mode = CMS_TCP_MODE_ELDIS,
		gboolean (* on_new_client) (cms_client_t *, void *) = NULL,
		void *on_new_client_data = NULL,
		gboolean (* on_delete_client) (cms_client_t *, void *) = NULL,
		void *on_delete_client_data = NULL)
	{
		server = cms_server_2_new (flags, addr.c_str (), port.c_str (), ping, 
			mode, CMS_CLIENT_MAX_BUFFER_SIZE, on_new_client, on_new_client_data, 
			on_delete_client, on_delete_client_data, NULL);
		if (!server) CONFIG_ERROR ("server2 errror");
	}
	
	CMSServer2 (
		int flags, 
		std::string addr, 
		unsigned short port, 
		unsigned ping, 
		enum CMS_TCP_MODE mode = CMS_TCP_MODE_ELDIS,
		gboolean (* on_new_client) (cms_client_t *, void *) = NULL,
		void *on_new_client_data = NULL,
		gboolean (* on_delete_client) (cms_client_t *, void *) = NULL,
		void *on_delete_client_data = NULL)
	{
		char strport[16];
		sprintf (strport, "%u", port);
		server = cms_server_2_new (flags, addr.c_str (), strport, ping, 
			mode, CMS_CLIENT_MAX_BUFFER_SIZE, on_new_client, on_new_client_data, 
			on_delete_client, on_delete_client_data, NULL);
		if (!server) CONFIG_ERROR ("server2 errror");
	}
	
	~CMSServer2 ()
	{
		cms_server_2_delete (server);
	}

	void Init ()
	{
		cms_server_2_init (server);
	}

	void ShutDown ()
	{
		cms_server_2_shutdown (server);
	}

	void Disconnect (unsigned connection_id)
	{
		cms_server_2_disconnect (server, connection_id);
	}

	bool IsConnected ()
	{
		return cms_server_2_is_connected (server);
	}

	std::string GetRemoteAddr ()
	{
		char * addr = cms_server_2_get_remote_addr (server, 0);
		std::string str (addr);
		g_free (addr);
		return str;
	}
	
	std::string GetRemoteAddr (unsigned connection_id)
	{
		char * addr = cms_server_2_get_remote_addr (server, connection_id);
		std::string str (addr);
		g_free (addr);
		return str;
	}
	
	int Send (const void * buffer, unsigned short size)
	{
		return cms_server_2_send (server, buffer, size, 0);
	}

	int Send (const void * buffer, unsigned short size, unsigned connection_id)
	{
		return cms_server_2_send (server, buffer, size, connection_id);
	}

	int Recv (void * buffer, unsigned short maxsize)
	{
		return cms_server_2_recv (server, buffer, maxsize, NULL);
	}

	int Recv (void * buffer, unsigned short maxsize, unsigned * connection_id)
	{
		return cms_server_2_recv (server, buffer, maxsize, connection_id);
	}

	unsigned GetRecvBufferFillSpace ()
	{
		return cms_server_2_get_recv_buffer_fill_space (server);
	}

	unsigned GetRecvBufferMessages ()
	{
		return cms_server_2_get_recv_buffer_messages (server);
	}

};

#endif /* _CMS_SERVER_2_HH */
