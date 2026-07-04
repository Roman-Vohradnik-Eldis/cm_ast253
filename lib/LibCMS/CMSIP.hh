#ifndef _CMS_IP_HH
#define _CMS_IP_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_ip.h"

class CMSIP
{
  private:
  	cms_ip_t * ip;

  public:
	CMSIP (int flags, std::string descriptors)
	{
		ip = cms_ip_new (flags, descriptors.c_str ());
		if (!ip) CONFIG_ERROR ("ip error");
	}
	
	~CMSIP ()
	{
		cms_ip_delete (ip);
	}

	void Init ()
	{
		cms_ip_init (ip);
	}
	
	void ShutDown ()
	{
		cms_ip_shutdown (ip);
	}

	bool IsConnected ()
	{
		return cms_ip_is_connected (ip);
	}
	
	std::string GetRemoteAddr ()
	{
		char * addr = cms_ip_get_remote_addr (ip);
		std::string str (addr);
		g_free (addr);
		return str;
	}
	
	unsigned GetRecvBufferFillSpace ()
	{
		return cms_ip_get_recv_buffer_fill_space (ip);
	}

	unsigned GetRecvBufferMessages ()
	{
		return cms_ip_get_recv_buffer_messages (ip);
	}
	
	int Send (const void * data, unsigned short size)
	{
		return cms_ip_send (ip, data, size);
	}

	int Recv (void * data, unsigned short maxsize)
	{
		return cms_ip_recv (ip, data, maxsize);
	}
};

#endif /* _CMS_IP_HH */
