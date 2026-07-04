#include "cms_debug.h"
#include "cms_local.h"

#define CMS_LOCAL_MAGIC 0x7206090B

cms_local_t * cms_local_new (const char * recvpath, const char * sendpath)
{
	int status;
	
	// alokujeme objekt
	cms_local_t * local = (cms_local_t *) g_malloc0 (sizeof (cms_local_t));
	if (!local)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujeme promenne
	local->socket = -1;
	if (recvpath && recvpath[0])
	{
		local->recv_addr.sun_family = AF_UNIX;
		strncpy (local->recv_addr.sun_path, recvpath, 100);
	}
	if (sendpath && sendpath[0])
	{
		local->send_addr.sun_family = AF_UNIX;
		strncpy (local->send_addr.sun_path, sendpath, 100);
	}

	// alokujeme socket
	local->socket = socket (PF_UNIX, SOCK_DGRAM, 0);
	if (local->socket == -1)
	{
		cms_ns_error ("libcms", "socket error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	// pripojime se na socket
	if (recvpath)
	{
		unlink (recvpath);
		status = bind (local->socket, (struct sockaddr *) 
			&local->recv_addr, sizeof (local->recv_addr));
		if (status == -1)
		{
			cms_ns_error ("libcms", "bind error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}
	}

	// nastavime magicke cislo
	local->magic = CMS_LOCAL_MAGIC;

	return local;

failed:
	if (local->socket != -1)
	{
		close (local->socket);
	}
	if (local)
	{
		g_free (local);
	}
	return NULL;
}

void cms_local_delete (cms_local_t * local)
{
	if (!local || local->magic != CMS_LOCAL_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", local->magic);
		return;
	}
	// uvolnime polozky objektu
	if (local->socket != -1)
	{
		close (local->socket);
	}

	// uvolnime objekt		
	g_free (local);
}

int cms_local_send (cms_local_t * local, const void * data, unsigned size)
{
	if (!local || local->magic != CMS_LOCAL_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", local->magic);
		return -1;
	}

	if (!local->send_addr.sun_family)
	{
		return -1;
	}
	
	return sendto (local->socket, data, size, 0, 
		(struct sockaddr *)&local->send_addr, sizeof (local->send_addr));
}

int cms_local_recv (cms_local_t * local, void * data, unsigned maxsize)
{
	if (!local || local->magic != CMS_LOCAL_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", local->magic);
		return -1;
	}

	if (!local->recv_addr.sun_family)
	{
		return -1;
	}

	return recv (local->socket, data, maxsize, 0);
}

