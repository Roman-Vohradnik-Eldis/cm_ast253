/*
 * Copyright (C) 2000 - 2003
 * ELDIS Pardubice, s.r.o.
 * Daniel Pirkl <pirkl@eldis.cz>
 */

#ifndef _CMS_CLIENT_HH
#define _CMS_CLIENT_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_client.h"

class CMSClient
{
	cms_client_t * client;
	
  public:
	CMSClient (
		int flags, 
		std::string addr, 
		std::string port, 
		unsigned ping,
		enum CMS_TCP_MODE mode = CMS_TCP_MODE_ELDIS,
		unsigned max_buffer_size = CMS_CLIENT_MAX_BUFFER_SIZE)
	{
		client = cms_client_new (flags, addr.c_str (), port.c_str (), ping, 
			mode, max_buffer_size, NULL);
		if (!client) CONFIG_ERROR ("client errror");	
	}

	CMSClient (
		int flags, 
		std::string addr, 
		unsigned short port, 
		unsigned ping,
		enum CMS_TCP_MODE mode = CMS_TCP_MODE_ELDIS,
		unsigned max_buffer_size = CMS_CLIENT_MAX_BUFFER_SIZE)
	{
		char strport[16];
		sprintf (strport, "%u", port);
		client = cms_client_new (flags, addr.c_str (), strport, ping, 
			mode, max_buffer_size, NULL);
		if (!client) CONFIG_ERROR ("client errror");	
	}
	
	CMSClient (cms_client_t * c)
	{
		client = c;
	}

	~CMSClient ()
	{
		cms_client_delete (client);
	}

	void Init ()
	{
		cms_client_init (client);
	}
	
	void ShutDown ()
	{
		cms_client_shutdown (client);
	}

	std::string getLocalAddr ()
	{
		return cms_client_get_local_addr (client);
	}

	unsigned short getLocalPort ()
	{
		return cms_client_get_local_port (client);
	}

	std::string getRemoteAddr ()
	{
		return cms_client_get_remote_addr (client);
	}

	unsigned short getRemotePort ()
	{
		return cms_client_get_remote_port (client);
	}

	bool IsConnected ()
	{
		return cms_client_is_connected (client);
	}

	bool Connect ()
	{
		return cms_client_connect (client);
	}

	void Disconnect ()
	{
		cms_client_disconnect (client);
	}
	
	void Flush ()
	{
		cms_client_flush (client);
	}

	int Send (const void * data, unsigned short size)
	{
		return cms_client_send (client, data, size);
	}

	int Recv (void * data, unsigned short maxsize)
	{
		return cms_client_recv (client, data, maxsize);
	}
};

#endif /* _CMS_CLIENT_HH */
