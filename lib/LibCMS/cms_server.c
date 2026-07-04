#include "cms_server.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_SERVER_MAGIC 0x72060907

extern void _cms_client_init ();

cms_server_t * cms_server_new (
	int send_recv_flags, 
	const char * addr, 
	const char * port,
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size)
{
	struct addrinfo hints;
	char tmp_port[16];
	char * ptr;
	int status;
	int one;

#ifdef G_OS_WIN32
	WSADATA data;
	WSAStartup (0x0101, &data);
#endif

	// inicializujeme globalni tcp zamky
	_cms_client_init ();

	// alokujeme objekt
	cms_server_t * server = (cms_server_t *) g_malloc0 (sizeof (cms_server_t));
	if (!server)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// iniciliazujeme promenne
	server->run = TRUE;
	server->address = g_strdup (addr);
	server->port = g_strdup (port);
	server->socket = -1;
	server->send_recv_flags = send_recv_flags;
	server->ping = ping;
	server->tcp_mode = mode;
	server->max_buffer_size = max_buffer_size;

	// upravime cislo portu, pokud je zadano hexadecimalne
	if (port[0] == '0' && port[1] == 'x')
	{
		status = strtol (port + 2, &ptr, 16);
		if (!ptr || *ptr)
		{
			cms_ns_error ("libcms", "wrong port number %s", port);
			goto failed;
		}
		sprintf (tmp_port, "%d", status);
	}
	else
	{
		strcpy (tmp_port, port);
	}

	// zjistime adresu serveru
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo (addr, tmp_port, &hints, &server->addrinfo);
	if (status != 0)
	{
		cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
			addr, port, status, errno, strerror (errno));
		goto failed;
	}

	// otevreme socket
	server->socket = socket (server->addrinfo->ai_family, 
		server->addrinfo->ai_socktype, server->addrinfo->ai_protocol);
	if (server->socket == -1)
	{
		cms_ns_error ("libcms", "socket error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

#ifdef G_OS_UNIX
	// nastavime parametry socketu
	status = fcntl (server->socket, F_SETFD, FD_CLOEXEC);
	if (status == -1)
	{
		cms_ns_error ("libcms", "fcntl F_SETFD error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}
#endif

	one = 1;
	status = setsockopt (server->socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof (one));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SO_KEEPALIVE error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	one = 1;
	status = setsockopt (server->socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof (one));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SO_REUSEADDR error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	// napojime socket na lokalni interface
	status = bind (server->socket, server->addrinfo->ai_addr, 
		server->addrinfo->ai_addrlen);
	if (status == -1)
	{
		cms_ns_error ("libcms", "bind error (%s,%s,%d,%s)", 
			addr, port, errno, strerror (errno));
		goto failed;
	}

	// specifikujeme, kolik spojeni muze cekat ve fronte
	status = listen (server->socket, 16);
	if (status == -1)
	{
		cms_ns_error ("libcms", "listen error");
		goto failed;
	}

	cms_ns_info ("libcms", "server is listening on local interface (%d)", 
		server->socket);

	server->magic = CMS_SERVER_MAGIC;
	return server;

failed:
	if (server)
	{
		if (server->socket >= 0)
		{
			close (server->socket);
		}
		if (server->addrinfo)
		{
			freeaddrinfo (server->addrinfo);
		}
		if (server->address)
		{
			g_free (server->address);
		}
		if (server->port)
		{
			g_free (server->port);
		}
		g_free (server);
	}
	
	return NULL;
}

void cms_server_delete (cms_server_t * server)
{
	if (!server || server->magic != CMS_SERVER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return;
	}

	cms_server_shutdown (server);

	server->magic = 0;
	
	if (server->socket >= 0)
	{
		close (server->socket);
	}
	if (server->addrinfo)
	{
		freeaddrinfo (server->addrinfo);
	}
	if (server->address)
	{
		g_free (server->address);
	}
	if (server->port)
	{
		g_free (server->port);
	}
	g_free (server);
}

void cms_server_shutdown (cms_server_t * server)
{
	if (!server || server->magic != CMS_SERVER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return;
	}

	if (server->socket >= 0)
	{
		server->run = FALSE;
		shutdown (server->socket, SHUT_RDWR);
	}
}

cms_client_t * cms_server_wait_for_client (cms_server_t * server)
{
	cms_client_t * client;
	int socket;
	int status;
	int one;

	if (!server || server->magic != CMS_SERVER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return 0;
	}

	// objekt jiz je mimi provoz
	if (!server->run)
	{
		return NULL;
	}

	// prijmeme pripojeni noveho klienta
	socket = accept (server->socket, NULL, NULL);
	if (socket == -1)
	{
		cms_ns_warning ("libcms", "accept error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}

#ifdef G_OS_UNIX
	// nastavime parametry socketu
	status = fcntl (socket, F_SETFD, FD_CLOEXEC);
	if (status == -1)
	{
		cms_ns_error ("libcms", "fcntl F_SETFD error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}
#endif

	one = 1;
	status = setsockopt (socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof (one));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SO_KEEPALIVE error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}

	// vytvorime spojeni na klienta
	client = cms_client_new_from_server (server->send_recv_flags, socket, 
		server->ping, server->tcp_mode, server->max_buffer_size, NULL, 0);
	if (!client)
	{
		return NULL;
	}

	// inicializujeme objekt
	cms_client_init (client);

	return client;
}
