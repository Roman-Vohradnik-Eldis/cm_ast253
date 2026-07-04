#ifndef _CMS_LOCAL_HH
#define _CMS_LOCAL_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_local.h"

class CMSLocal
{
	cms_local_t * local;

  public:
  	CMSLocal (string recv, string send)
	{
		local = cms_local_new (recv.c_str (), send.c_str ());
		if (!local) CONFIG_ERROR ("local errror");
	}

	~CMSLocal ()
	{
		cms_local_delete (local);
	}

	int Send (const void * data, unsigned size)
	{
		return cms_local_send (local, data, size);
	}

	int Recv (void * data, unsigned maxsize)
	{
		return cms_local_recv (local, data, maxsize);
	}
};

#endif /* _CMS_LOCAL_HH */
