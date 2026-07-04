#ifndef _CMS_UDP_HH
#define _CMS_UDP_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_udp.h"

class CMSUDP
{
	cms_udp_t * udp;

  public:
	CMSUDP (
		int init_flags, 
		string recv_addr, 
		string recv_port, 
		string send_addr, 
		string send_port)
	{
		udp = cms_udp_new (init_flags, recv_addr.c_str (), recv_port.c_str (),
			send_addr.c_str (), send_port.c_str (), NULL, NULL);
		if (!udp) CONFIG_ERROR ("udp errror");
	}

	~CMSUDP ()
	{
		cms_udp_delete (udp);
	}

	void Init ()
	{
		cms_udp_init (udp);
	}

	void ShutDown ()
	{
		cms_udp_shutdown (udp);
	}

	void Flush ()
	{
		cms_udp_flush (udp);
	}

	int Send (const void * data, unsigned short size)
	{
		return cms_udp_send (udp, data, size);
	}

	int Recv (void * data, unsigned short maxsize)
	{
		return cms_udp_recv (udp, data, maxsize);
	}
};

#endif /* _CMS_UDP_HH */
