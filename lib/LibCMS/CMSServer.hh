#ifndef _CMS_SERVER_H
#define _CMS_SERVER_H

#include "CMSHeader.hh"
#include "CMSClient.hh"
#include "CMSDebug.hh"
#include "cms_server.h"

class CMSServer
{
	cms_server_t * server;

  public:
	CMSServer(
		int init_flags, 
		std::string addr, 
		std::string port, 
		unsigned ping, 
		enum CMS_TCP_MODE mode = CMS_TCP_MODE_ELDIS,
		unsigned max_buffer_size = CMS_CLIENT_MAX_BUFFER_SIZE)
	{
		server = cms_server_new (init_flags, addr.c_str (), port.c_str (), ping, 
			mode, max_buffer_size);
		if (!server) CONFIG_ERROR ("server errror");
	}
	
	CMSServer(
		int init_flags, 
		std::string addr, 
		unsigned short port, 
		unsigned ping, 
		enum CMS_TCP_MODE mode = CMS_TCP_MODE_ELDIS,
		unsigned max_buffer_size = CMS_CLIENT_MAX_BUFFER_SIZE)
	{
		char strport[16];
		sprintf (strport, "%u", port);
		server = cms_server_new (init_flags, addr.c_str (), strport, ping, 
			mode, max_buffer_size);
		if (!server) CONFIG_ERROR ("server errror");
	}
	
	~CMSServer()
	{
		cms_server_delete (server);
	}

	CMSClient * WaitForClient()
	{
		cms_client_t * client = cms_server_wait_for_client (server);
		if (!client) return NULL;
		return new CMSClient (client);
	}
	
	void ShutDown ()
	{
		cms_server_shutdown (server);
	}
};

#endif /* _C_TCP_SERVER_H */
