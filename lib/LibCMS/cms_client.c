#include "cms_client.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_CLIENT_MAGIC 0x72060902

void _cms_client_epoll_add (cms_client_t * client);
void _cms_client_epoll_remove (cms_client_t * client);
void _cms_client_epoll_set (cms_client_t * client);
gpointer cms_client_epoll_thread (gpointer data);
gpointer cms_client_ping_thread (gpointer data);

static GMutex * client_epoll_mutex = NULL;
static GCond * client_epoll_cond = NULL;
static GThread * client_epoll_thread = NULL;
static GList * client_epoll_list = NULL;
static int client_epoll_fd = -1;

static GThread * client_ping_thread = NULL;
static int client_ping_fd = -1;
static int client_ping_sequence = 0;

void _cms_client_init ()
{
	if (!client_epoll_mutex)
	{
		client_epoll_mutex = g_mutex_new ();
	}
	if (!client_epoll_cond)
	{
		client_epoll_cond = g_cond_new ();
	}
}

cms_client_t * cms_client_new (
	int send_recv_flags, 
	const char * addr, 
	const char * port,
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size,
	cms_round_buffer_t * recv_buffer)
{
	struct addrinfo hints;
	char tmp_port[16];
	char * ptr;
	int status;
	
#ifdef G_OS_WIN32
	WSADATA data;
	WSAStartup (0x0101, &data);
#endif

	// inicializujeme globalni tcp zamky
	_cms_client_init ();
	
	// alokujeme objekt
	cms_client_t * client = (cms_client_t *) g_malloc0 (sizeof (cms_client_t));
	if (!client)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujeme promenne
	client->server = FALSE;
	client->init = FALSE;
	client->connected = FALSE;
	client->socket = -1;
	client->socket2 = -1;
	client->port = g_strdup (port);
	client->send_recv_flags = send_recv_flags;
	client->tcp_mode = mode;
	client->ping_timeout = (ping & 0xff) * 10;
	client->ping_interval = (ping >> 8) & 0xff;
	client->ping_type = (ping >> 16) & 0xff;
	if (!client->ping_interval)
	{
		client->ping_interval = ping & 0xff;
	}

	// vytvorime mutext na navazovani spojeni	
	client->mutex = g_mutex_new ();
	if (!client->mutex)
	{
		cms_ns_error ("libcms", "g_mutex_new error");
		goto failed;
	}

	// alokujeme buffer na vyslani jedne zpravy
	client->send_data = g_malloc (MSGMAX + 2);
	if (!client->send_data)
	{
		cms_ns_error ("libcms", "g_malloc error");
		goto failed;
	}
	
	// alokujeme buffer na prijem jedne zpravy
	client->recv_data = g_malloc (MSGMAX + 2);
	if (!client->recv_data)
	{
		cms_ns_error ("libcms", "g_malloc error");
		goto failed;
	}
	
	// vytvorime buffer na vysilani
	if ((send_recv_flags & CMS_FLAG_SEND) || client->ping_type)
	{
		if (mode == CMS_TCP_MODE_RAW)
		{
			client->send_buffer = cms_round_buffer_new ("client send", 
				max_buffer_size, CMS_ROUND_BUFFER_WRITE_GROW);
			if (!client->send_buffer)
			{
				cms_ns_error ("libcms", "cms_round_buffer_new_with_block error");
				goto failed;
			}
		}
		else
		{
			client->send_buffer = cms_round_buffer_new_with_block ("client send", 
				max_buffer_size, CMS_ROUND_BUFFER_WRITE_GROW);
			if (!client->send_buffer)
			{
				cms_ns_error ("libcms", "cms_round_buffer_new_with_block error");
				goto failed;
			}
		}
	}

	// vytvorime buffer na prijem
	if (send_recv_flags & CMS_FLAG_RECV)
	{
		if (recv_buffer)
		{
			client->external_recv_buffer = recv_buffer;
		}
		else if (mode == CMS_TCP_MODE_RAW)
		{
			client->recv_buffer = cms_round_buffer_new ("client recv", 
				max_buffer_size, CMS_ROUND_BUFFER_READ_BLOCK 
				| CMS_ROUND_BUFFER_WRITE_GROW);
			if (!client->recv_buffer)
			{
				cms_ns_error ("libcms", "cms_round_buffer_new error");
				goto failed;
			}
		}
		else
		{
			client->recv_buffer = cms_round_buffer_new_with_block ("client recv", 
				max_buffer_size, CMS_ROUND_BUFFER_READ_BLOCK 
				| CMS_ROUND_BUFFER_WRITE_GROW);
			if (!client->recv_buffer)
			{
				cms_ns_error ("libcms", "cms_round_buffer_new_with_block error");
				goto failed;
			}
		}
	}

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
	status = getaddrinfo (addr, tmp_port, &hints, &client->addrinfo);
	if (status != 0)
	{
		cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
			addr, port, status, errno, gai_strerror (status));
		goto failed;
	}

	// odkaz na adresu
	client->sockaddr = client->addrinfo->ai_addr;
	client->sockaddrlen = client->addrinfo->ai_addrlen;
	
	// ulozime vzdalenout adresu
	if (client->sockaddr->sa_family == AF_INET)
	{
		struct sockaddr_in * addr_in = (struct sockaddr_in *) client->sockaddr;
		inet_ntop (AF_INET, &addr_in->sin_addr, client->remote_addr, INET_ADDRSTRLEN);
		client->remote_port = ntohs (addr_in->sin_port);
		client->loopback = (INADDR_LOOPBACK == ntohl (addr_in->sin_addr.s_addr));
	}
	else if (client->sockaddr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) client->sockaddr;
		inet_ntop (AF_INET6, &addr_in6->sin6_addr, client->remote_addr, INET6_ADDRSTRLEN);
		client->remote_port = ntohs (addr_in6->sin6_port);
		client->loopback = IN6_IS_ADDR_LOOPBACK (addr_in6);
//		client->loopback = IN6_IS_ADDR_LOOPBACK (addr_in6->sin6_addr.s6_addr);
	}

	// nastavime magicke cislo
	client->magic = CMS_CLIENT_MAGIC;

	// vratime objekt
	return client;

failed:
	if (client)
	{
		if (client->mutex)
		{
			g_mutex_free (client->mutex);
		}
		if (client->port)
		{
			g_free (client->port);
		}
		if (client->addrinfo)
		{
			freeaddrinfo (client->addrinfo);
		}
		if (client->send_data)
		{
			g_free (client->send_data);
		}
		if (client->recv_data)
		{
			g_free (client->recv_data);
		}
		if (client->send_buffer)
		{
			cms_round_buffer_delete (client->send_buffer);
		}
		if (client->recv_buffer)
		{
			cms_round_buffer_delete (client->recv_buffer);
		}
		g_free (client);
	}
	
	return NULL;
}

cms_client_t * cms_client_new_from_server (
	int send_recv_flags, 
	int socket,
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size,
	cms_round_buffer_t * recv_buffer,
	unsigned connection_id)
{
	struct sockaddr * addr;
	socklen_t addrlen;
	char addrbuf[64];
	int status;

	// alokujeme client objekt
	cms_client_t * client = (cms_client_t *) g_malloc0 (sizeof (cms_client_t));
	if (!client)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujeme promenne
	client->server = TRUE;
	client->init = FALSE;
	client->connected = FALSE;
	client->socket = socket;
	client->socket2 = -1;
	client->connection_id = connection_id;
	client->send_recv_flags = send_recv_flags;
	client->tcp_mode = mode;
	client->ping_timeout = (ping & 0xff) * 10;
	client->ping_interval = (ping >> 8) & 0xff;
	client->ping_type = (ping >> 16) & 0xff;
	if (!client->ping_interval)
	{
		client->ping_interval = ping & 0xff;
	}

	// vytvorime mutext na navazovani spojeni	
	client->mutex = g_mutex_new ();
	if (!client->mutex)
	{
		cms_ns_error ("libcms", "g_mutex_new error");
		goto failed;
	}

	// alokujeme buffer na vyslani jedne zpravy
	client->send_data = g_malloc (MSGMAX + 2);
	if (!client->send_data)
	{
		cms_ns_error ("libcms", "g_malloc error");
		goto failed;
	}
	
	// alokujeme buffer na prijem jedne zpravy
	client->recv_data = g_malloc (MSGMAX + 2);
	if (!client->recv_data)
	{
		cms_ns_error ("libcms", "g_malloc error");
		goto failed;
	}
	
	// vytvorime buffer na vysilani
	if ((send_recv_flags & CMS_FLAG_SEND) || client->ping_type)
	{
		client->send_buffer = cms_round_buffer_new_with_block ("client send", 
			max_buffer_size, CMS_ROUND_BUFFER_WRITE_GROW);
		if (!client->send_buffer)
		{
			cms_ns_error ("libcms", "cms_round_buffer_new_with_block error");
			goto failed;
		}
	}

	// vytvorime buffer na prijem
	if (send_recv_flags & CMS_FLAG_RECV)
	{
		if (recv_buffer)
		{
			client->external_recv_buffer = recv_buffer;
		}
		else
		{
			client->recv_buffer = cms_round_buffer_new_with_block ("client recv", 
				max_buffer_size, CMS_ROUND_BUFFER_READ_BLOCK 
				| CMS_ROUND_BUFFER_WRITE_GROW);
			if (!client->recv_buffer)
			{
				cms_ns_error ("libcms", "cms_round_buffer_new_with_block error");
				goto failed;
			}
		}
	}
	
	// zjistime vzdalenou adresu
	addr = (struct sockaddr *) addrbuf;
	addrlen = sizeof (addrbuf);
	status = getpeername (client->socket, addr, &addrlen);
	if (status != 0)
	{
		cms_ns_error ("libcms", "getpeername error (%d,%d,%d,%s)", 
			client->socket, status, errno, strerror (errno));
		goto failed;
	}

	// odkaz na adresu
	client->sockaddr = g_memdup (addr, addrlen);
	client->sockaddrlen = addrlen;

	// ulozime vzdalenou adresu
	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in * addr_in = (struct sockaddr_in *) addr;
		inet_ntop (AF_INET, &addr_in->sin_addr, client->remote_addr, INET_ADDRSTRLEN);
		client->remote_port = ntohs (addr_in->sin_port);
		client->loopback = (INADDR_LOOPBACK == ntohl (addr_in->sin_addr.s_addr));
	}
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) addr;
		inet_ntop (AF_INET6, &addr_in6->sin6_addr, client->remote_addr, INET6_ADDRSTRLEN);
		client->remote_port = ntohs (addr_in6->sin6_port);
		client->loopback = IN6_IS_ADDR_LOOPBACK (addr_in6);
//		client->loopback = IN6_IS_ADDR_LOOPBACK (addr_in6->sin6_addr.s6_addr);
	}

	// zjistime lokalni adresu
	addr = (struct sockaddr *) addrbuf;
	addrlen = sizeof (addrbuf);
	status = getsockname (client->socket, addr, &addrlen);
	if (status != 0)
	{
		cms_ns_error ("libcms", "getsockname error (%d,%d,%d,%s)", 
			client->socket, status, errno, strerror (errno));
		goto failed;
	}

	// ulozime lokalni adresu
	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in * addr_in = (struct sockaddr_in *) addr;
		inet_ntop (AF_INET, &addr_in->sin_addr, client->local_addr, INET_ADDRSTRLEN);
		client->local_port = ntohs (addr_in->sin_port);
	}
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) addr;
		inet_ntop (AF_INET6, &addr_in6->sin6_addr, client->local_addr, INET6_ADDRSTRLEN);
		client->local_port = ntohs (addr_in6->sin6_port);
	}

	cms_ns_info ("libcms", "new connection (%d) %s:%x:%d <-> %s:%x:%d", 
		client->socket, client->local_addr, client->local_port, client->local_port,
		client->remote_addr, client->remote_port, client->remote_port);

	// nastavime magicke cislo
	client->magic = CMS_CLIENT_MAGIC;

	// pridame objekt do epoolu
	client->connected = TRUE;
	_cms_client_epoll_add (client);

	// vratime objekt
	return client;

failed:
	if (client)
	{
		if (client->mutex)
		{
			g_mutex_free (client->mutex);
		}
		if (client->port)
		{
			g_free (client->port);
		}
		if (client->sockaddr)
		{
			g_free (client->sockaddr);
		}
		if (client->send_data)
		{
			g_free (client->send_data);
		}
		if (client->recv_data)
		{
			g_free (client->recv_data);
		}
		if (client->send_buffer)
		{
			cms_round_buffer_delete (client->send_buffer);
		}
		if (client->recv_buffer)
		{
			cms_round_buffer_delete (client->recv_buffer);
		}
		g_free (client);
	}
	
	return NULL;
}

void cms_client_delete (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return;
	}

	// ukoncime komunikaci
	cms_client_shutdown (client);

	// odstranime objekt z pollu
	_cms_client_epoll_remove (client);

	// smazeme magicke cislo
	client->magic = 0;

	// uvolnime polozky objektu
	if (client->socket != -1)
	{
		close (client->socket);
	}
	if (client->socket2 != -1)
	{
		close (client->socket2);
	}
	if (client->port)
	{
		g_free (client->port);
	}
	if (client->addrinfo)
	{
		freeaddrinfo (client->addrinfo);
	}
	else if (client->sockaddr)
	{
		g_free (client->sockaddr);
	}
	if (client->mutex)
	{
		g_mutex_free (client->mutex);
	}
	if (client->send_data)
	{
		g_free (client->send_data);
	}
	if (client->recv_data)
	{
		g_free (client->recv_data);
	}
	if (client->send_buffer)
	{
		cms_round_buffer_delete (client->send_buffer);
	}
	if (client->recv_buffer)
	{
		cms_round_buffer_delete (client->recv_buffer);
	}

	// uvolnime objekt	
	g_free (client);
}

void cms_client_init (cms_client_t * client)
{
	int status;
	
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return;
	}

	// zamkneme zamek epollu
	g_mutex_lock (client_epoll_mutex);

	// poznamename provedeni inicializace
	client->init = TRUE;
	
	// vytvorime epoll
	if (client_epoll_fd == -1)
	{
		client_epoll_fd = epoll_create (1);
		if (client_epoll_fd == -1)
		{
			cms_ns_error ("libcms", "epoll_create error (%d,%s)", 
				errno, strerror (errno));
		}
		status = fcntl (client_epoll_fd, F_SETFD, FD_CLOEXEC);
		if (status == -1)
		{
			cms_ns_error ("libcms", "fcntl F_SETFD FD_CLOEXEC error (%d,%s)", 
				errno, strerror (errno));
		}
		cms_ns_info ("libcms", "epoll_create (%d)", client_epoll_fd);
	}

	// sputime epoll thread
	if (!client_epoll_thread)
	{
		client_epoll_thread = g_thread_create_full (
			cms_client_epoll_thread, NULL, 0x100000,
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);
		if (!client_epoll_thread)
		{
			cms_ns_error ("libcms", "g_thread_create_full error");
		}
	}

	// sputime ping thread
	if (!client_ping_thread)
	{
		client_ping_thread = g_thread_create_full (
			cms_client_ping_thread, NULL, 0x100000,
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);
		if (!client_ping_thread)
		{
			cms_ns_error ("libcms", "g_thread_create_full error");
		}
	}

	// odemkneme zamek epollu
	g_mutex_unlock (client_epoll_mutex);
}

void cms_client_shutdown (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return;
	}

	// zamkneme zamek pro konekce
	g_mutex_lock (client->mutex);

	client->connected = FALSE;

	// ukoncime cinnost socketu
	if (client->socket != -1)
	{
		shutdown (client->socket, SHUT_RDWR);
	}
	if (client->socket2 != -1)
	{
		shutdown (client->socket2, SHUT_RDWR);
	}

	// ukoncime cinnost vysilaciho buffer
	if (client->send_buffer)
	{
		cms_round_buffer_set_flags (client->send_buffer, 0);
		cms_round_buffer_clear (client->send_buffer);
	}

	// ukoncime cinnost prijimaciho bufferu
	if (client->recv_buffer)
	{
		cms_round_buffer_set_flags (client->recv_buffer, 0);
		cms_round_buffer_clear (client->recv_buffer);
	}

	// odemkneme zamek pro konekce
	g_mutex_unlock (client->mutex);
}

const char * cms_client_get_local_addr (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return NULL;
	}

	return client->local_addr;
}

unsigned short cms_client_get_local_port (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return 0;
	}

	return client->local_port;
}

const char * cms_client_get_remote_addr (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return NULL;
	}

	return client->remote_addr;
}

unsigned short cms_client_get_remote_port (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return 0;
	}

	return client->remote_port;
}

unsigned short cms_client_get_connection_id (cms_client_t * client)
{
	return client->connection_id;
}

gboolean cms_client_is_connected (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return FALSE;
	}

	return client->connected;
}

gboolean cms_client_connect (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return FALSE;
	}

	struct sockaddr * addr;
	socklen_t addrlen;
	char tmp[64];
	int status;
	int one = 1;

	if (client->server)
	{
		cms_ns_error ("libcms", "this connection can't be connected again");
		return FALSE;
	}

	// odstranime objekt z pollu
	_cms_client_epoll_remove (client);

	// zamkneme zamek pro konekce
	g_mutex_lock (client->mutex);

	// otevreme socket
	if (client->socket2 == -1)
	{
		client->socket2 = socket (client->addrinfo->ai_family, 
			client->addrinfo->ai_socktype, client->addrinfo->ai_protocol);
		if (client->socket2 == -1)
		{
			cms_ns_error ("libcms", "socket error - unable to create socket (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}

#ifdef G_OS_UNIX
		// nastavime parametry socketu
		status = fcntl (client->socket2, F_SETFD, FD_CLOEXEC);
		if (status == -1)
		{
			cms_ns_error ("libcms", "fcntl F_SETFD error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}
#endif

		status = setsockopt (client->socket2, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof (one));
		if (status == -1)
		{
			cms_ns_error ("libcms", "setsockopt SO_KEEPALIVE error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}
	}

	// odemkneme zamek pro konekce
	g_mutex_unlock (client->mutex);

	// inicializujeme na socket spojeni na servru
	status = connect (client->socket2, client->sockaddr, client->sockaddrlen);
	if (status == -1)
	{
		return FALSE;
	}

	// zamkneme zamek pro konekce
	g_mutex_lock (client->mutex);

	// zjistime lokalni adresu
	addr = (struct sockaddr *) tmp;
	addrlen = sizeof (tmp);
	status = getsockname (client->socket2, addr, &addrlen);
	if (status != 0)
	{
		cms_ns_error ("libcms", "getsockname error (%d,%d,%d,%s)", 
			client->socket2, status, errno, strerror (errno));
		goto failed;
	}

	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in * addr_in = (struct sockaddr_in *) addr;
		inet_ntop (AF_INET, &addr_in->sin_addr, client->local_addr, INET_ADDRSTRLEN);
		client->local_port = ntohs (addr_in->sin_port);
	}
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) addr;
		inet_ntop (AF_INET6, &addr_in6->sin6_addr, client->local_addr, INET6_ADDRSTRLEN);
		client->local_port = ntohs (addr_in6->sin6_port);
	}

	// vynulujeme citac na prijem dat
	client->recv_time_count = 0;

	// zavreme stary socket	a dosadime novy
	int s = client->socket;
	client->socket = client->socket2;
	client->socket2 = -1;
	if (s >= 0)
	{
		close (s);
	}

	if (client->send_buffer)
	{
		cms_round_buffer_set_flags (client->send_buffer, 
			CMS_ROUND_BUFFER_WRITE_GROW);
	}

	if (client->recv_buffer)
	{
		cms_round_buffer_set_flags (client->recv_buffer, CMS_ROUND_BUFFER_READ_BLOCK 
			| CMS_ROUND_BUFFER_WRITE_GROW);
	}

	cms_ns_info ("libcms", "new connection (%d) %s:%x:%d <-> %s:%x:%d", 
		client->socket, client->local_addr, client->local_port, client->local_port, 
		client->remote_addr, client->remote_port, client->remote_port);

	// pridame objekt do epollu
	client->connected = TRUE;
	_cms_client_epoll_add (client);

	// odemkneme zamek pro konekce
	g_mutex_unlock (client->mutex);

	return TRUE;

failed:
	// odemkneme zamek pro konekce
	g_mutex_unlock (client->mutex);

	return FALSE;
}

void cms_client_disconnect (cms_client_t * client)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return;
	}

	// zamkneme zamek pro konekce
	g_mutex_lock (client->mutex);

	if (client->connected)
	{
		cms_ns_info ("libcms", "shutdown connection (%d) %s:%x:%d <-> %s:%x:%d", 
			client->socket, client->local_addr, client->local_port, client->local_port,
			client->remote_addr, client->remote_port, client->remote_port);
		client->connected = FALSE;
	}

	if (client->socket != -1)
	{
		shutdown (client->socket, SHUT_RDWR);
	}		

	if (client->socket2 != -1)
	{
		shutdown (client->socket2, SHUT_RDWR);
	}
	
	if (client->send_buffer)
	{
		cms_round_buffer_set_flags (client->send_buffer, 0);
		cms_round_buffer_clear (client->send_buffer);
	}

	if (client->recv_buffer)
	{
		cms_round_buffer_set_flags (client->recv_buffer, 0);
		cms_round_buffer_clear (client->recv_buffer);
	}
	
	// odemkneme zamek pro konekce
	g_mutex_unlock (client->mutex);
}

void cms_client_flush (cms_client_t * client)
{
	// zamkneme zamek
	g_mutex_lock (client_epoll_mutex);

	// cekame na vyprazdneni vystupniho bufferu
	while (client->pollout)
	{
		g_cond_wait (client_epoll_cond, client_epoll_mutex);
	}

	// odemkneme zamek
	g_mutex_unlock (client_epoll_mutex);
}

int cms_client_send (cms_client_t * client, const void * data, unsigned size)
{
	int status;
	
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return -1;
	}

	// objekt neni inicializovany na vysilani
	if (!(client->send_recv_flags & CMS_FLAG_SEND))
	{
		cms_ns_error ("libcms", "object is not initialized for sending %s:%x:%d %s:%x:%d",
			client->local_addr, client->local_port, client->local_port, 
			client->remote_addr, client->remote_port, client->remote_port);
		return -1;
	}
	
	// pokud je treba, inicializujeme objekt
	if (!client->init)
	{
		cms_client_init (client);
	}

	// neni navazane spojeni, vratime chybu
	if (!client->connected)
	{
		return -1;
	}

	// zapiseme data do bufferu
	status = cms_round_buffer_write (client->send_buffer, data, size);
	if (status != size)
	{
		cms_ns_error ("libcms", "cms_round_buffer_write error (%d,%d)", size, status);
		cms_client_disconnect (client);
		return status;
	}

	// povolime vysilani dat
	_cms_client_epoll_set (client);

	return status;
}

int cms_client_recv (cms_client_t * client, void * data, unsigned maxsize)
{
	if (!client || client->magic != CMS_CLIENT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", client->magic);
		return -1;
	}

	// objekt neni inicializovan na prijem
	if (!(client->send_recv_flags & CMS_FLAG_RECV))
	{
		cms_ns_error ("libcms", "object is not initialized for receiving %s:%x:%d %s:%x:%d",
			client->local_addr, client->local_port, client->local_port,
			client->remote_addr, client->remote_port, client->remote_port);
		return 0;
	}

	// objekt neni inicializovan na prime cteni
	if (client->external_recv_buffer)
	{
		return -1;
	}

	// pokud je treba, inicializujeme objekt
	if (!client->init)
	{
		cms_client_init (client);
	}

	// neni navazane spojeni, vratime chybu
	if (!client->connected)
	{
		return -1;
	}

	// prijimame v textovem rezimu, zakoncime nulou
	if (client->tcp_mode == CMS_TCP_MODE_TEXT)
	{
		int size = cms_round_buffer_read (client->recv_buffer, 
			data, maxsize > 0 ? maxsize - 1 : 0);
		if (size >= 0 && maxsize > 0)
		{
			((char *)data)[size] = 0;
		}
		return size;
	}

	return cms_round_buffer_read (client->recv_buffer, data, maxsize);
}

void _cms_client_send_eldis (cms_client_t * client, const void * data, unsigned size)
{
	int status;
	
	// ve vysilacim bufferu uz jsou data
	if (client->send_data_size)
	{
		if (data || size)
		{
			cms_ns_error ("libcms", "internal error %d %d", 
				size, client->send_data_size);
		}
	}

	// zkopirujeme nova data do vysilaciho bufferu
	else
	{
		client->send_data[0] = (size & 0xff);
		client->send_data[1] = (size >> 8) & 0xff;
		memcpy (client->send_data + 2, data, size);
		client->send_data_size = size + 2;
	}

	// odesleme data
	status = send (client->socket, client->send_data, 
		client->send_data_size, MSG_DONTWAIT | MSG_NOSIGNAL);
	//cms_ns_debug ("libcms", "send %d %d", client->send_data_size, status);
	
	if (status == -1)
	{
		if (errno == ECONNRESET)
		{
			cms_ns_info ("libcms", "send error (%d,%d,%d,%s)", 
				client->socket, status, errno, strerror (errno));
		}
		else
		{
			cms_ns_error ("libcms", "send error (%d,%d,%d,%s)", 
				client->socket, status, errno, strerror (errno));
		}
		
		cms_client_disconnect (client);
		return;
	}

	if (status == 0)
	{
		cms_ns_info ("libcms", "connection was broken (%d)", client->socket);
		cms_client_disconnect (client);
		return;
	}

	if (status < 0 || status > client->send_data_size)
	{
		cms_ns_error ("libcms", "send failed (%d,%d,%d)", 
			client->socket, status, client->send_data_size);
		client->send_data_size = 0;
		return;
	}
	
	// data byla odeslana
	client->send_data_size -= status;
	memmove (client->send_data, client->send_data + status, client->send_data_size);
}

int _cms_client_recv_eldis (cms_client_t * client, void * data, unsigned maxsize)
{
	unsigned size, size2;
	int status;

	while (client->recv_data_size < 2)
	{
		status = recv (client->socket, client->recv_data + client->recv_data_size, 
			2 - client->recv_data_size, MSG_DONTWAIT | MSG_NOSIGNAL);
		//cms_ns_debug ("libcms", "recv %d %d", client->recv_data_size, status);
		
		if (status == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0)
			{
				return -1;
			}
			
			if (errno == ECONNRESET)
			{
				cms_ns_info ("libcms", "recv error (%d,%d,%d,%s)", 
					client->socket, status, errno, strerror (errno));
			}
			else
			{
				cms_ns_error ("libcms", "recv error (%d,%d,%d,%s)", 
					client->socket, status, errno, strerror (errno));
			}

			cms_client_disconnect (client);
			return -1;
		}

		if (status == 0)
		{
			cms_ns_info ("libcms", "connection was broken (%d)", 
				client->socket);
			cms_client_disconnect (client);
			return -1;
		}

		client->recv_data_size += status;
	}

	size = ((unsigned) client->recv_data[0]) 
		| (((unsigned) client->recv_data[1]) << 8);
	if (size > maxsize)
	{
		cms_ns_warning ("libcms", "message is too long (%d,%d,%d)", 
			client->socket, size, maxsize);
		return -1;
	}

	// precteme vlastni datadata
	size2 = size + 2;
	while (client->recv_data_size < size2)
	{
		status = recv (client->socket, client->recv_data + client->recv_data_size,
			size2 - client->recv_data_size, MSG_DONTWAIT | MSG_NOSIGNAL);
		//cms_ns_debug ("libcms", "recv %d %d %d", size2, client->recv_data_size, status);
		
		if (status == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0)
			{
				return -1;
			}
			
			if (errno == ECONNRESET)
			{
				cms_ns_info ("libcms", "recv error (%d,%d,%d,%d,%s)", 
					client->socket, size2 - client->recv_data_size, 
					status, errno, strerror (errno));
			}
			else
			{
				cms_ns_error ("libcms", "recv error (%d,%d,%d,%d,%s)", 
					client->socket, size2 - client->recv_data_size, 
					status, errno, strerror (errno));
			}

			cms_client_disconnect (client);
			return -1;
		}

		if (status == 0)
		{
			cms_ns_info ("libcms", "connectin was broken (%d)", 
				client->socket);
			cms_client_disconnect (client);
			return -1;
		}

		client->recv_data_size += status;
	}

	// byla prijata zprava
	client->recv_data_size = 0;

	// poznamename si cas
	client->recv_time_count = 0;

	// zachytime pinkaci zpravu
	if (client->ping_type == CMS_PING_TYPE_DATA)
	{
		if (size == 0)
		{
			return -1;
		}
	}

	// zachytime starou pinkaci zpravu
	else if (client->ping_type == CMS_PING_TYPE_OLD)
	{
		if (size == 3
			&& client->recv_data[2] == 0
			&& client->recv_data[3] == 0
			&& client->recv_data[4] == 0)
		{
			return -1;
		}
	}

	// vratime pocet prijatych dat
	memcpy (data, client->recv_data + 2, size);
	return size;
}

void _cms_client_send_raw (cms_client_t * client, const void * data, unsigned size)
{
	int status;
	
	// ve vysilacim bufferu uz jsou data
	if (client->send_data_size)
	{
		if (data || size)
		{
			cms_ns_error ("libcms", "internal error %d %d", 
				size, client->send_data_size);
		}
	}

	// zkopirujeme nova data do vysilaciho bufferu
	else
	{
		memcpy (client->send_data, data, size);
		client->send_data_size = size;
	}

	// odesleme data
	status = send (client->socket, client->send_data, 
		client->send_data_size, MSG_DONTWAIT | MSG_NOSIGNAL);
	//cms_ns_debug ("libcms", "send %d %d", client->send_data_size, status);
	
	if (status == -1)
	{
		if (errno == ECONNRESET)
		{
			cms_ns_info ("libcms", "send error (%d,%d,%d,%s)", 
				client->socket, status, errno, strerror (errno));
		}
		else
		{
			cms_ns_error ("libcms", "send error (%d,%d,%d,%s)", 
				client->socket, status, errno, strerror (errno));
		}
		
		cms_client_disconnect (client);
		return;
	}

	if (status == 0)
	{
		cms_ns_info ("libcms", "connection was broken (%d)", 
			client->socket);
		cms_client_disconnect (client);
		return;
	}

	if (status < 0 || status > client->send_data_size)
	{
		cms_ns_error ("libcms", "send failed (%d,%d,%d)", 
			client->socket, status, client->send_data_size);
		client->send_data_size = 0;
		return;
	}

	// data byla odeslana
	client->send_data_size -= status;
	memmove (client->send_data, client->send_data + status, client->send_data_size);
}

int _cms_client_recv_raw (cms_client_t * client, void * data, unsigned maxsize)
{
	int status;

	// precteme vlastni datadata
	status = recv (client->socket, (char *) data, maxsize, MSG_DONTWAIT | MSG_NOSIGNAL);
	//cms_ns_debug ("libcms", "recv %d", status);

	if (status == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0)
		{
			return -1;
		}
		if (errno == ECONNRESET)
		{
			cms_ns_info ("libcms", "recv error (%d,%d,%d,%d,%s)", 
				client->socket, maxsize, status, errno, strerror (errno));
		}
		else
		{
			cms_ns_error ("libcms", "recv error (%d,%d,%d,%d,%s)", 
				client->socket, maxsize, status, errno, strerror (errno));
		}
		
		cms_client_disconnect (client);
		return -1;
	}

	if (status == 0)
	{
		cms_ns_info ("libcms", "connectin was broken (%d)", client->socket);
		cms_client_disconnect (client);
		return -1;
	}

	// poznamename si cas
	client->recv_time_count = 0;

	// vratime pocet prijatych dat
	return status;
}

void _cms_client_send_text (cms_client_t * client, const char * data, unsigned size)
{
	int status;
	
	// ve vysilacim bufferu uz jsou data
	if (client->send_data_size)
	{
		if (data || size)
		{
			cms_ns_error ("libcms", "internal error %d %d", 
				size, client->send_data_size);
		}
	}

	// zkopirujeme nova data do vysilaciho bufferu
	else
	{
		memcpy (client->send_data, data, size);
		client->send_data[size] = '\n';
		client->send_data_size = size + 1;
	}

	// odesleme data vcetne hlavicky
	status = send (client->socket, client->send_data, 
		client->send_data_size, MSG_DONTWAIT | MSG_NOSIGNAL);
	//cms_ns_debug ("libcms", "send %d %d", client->send_data_size, status);

	if (status == -1)
	{
		if (errno == ECONNRESET)
		{
			cms_ns_info ("libcms", "send error (%d,%d,%d,%s)", 
				client->socket, status, errno, strerror (errno));
		}
		else
		{
			cms_ns_error ("libcms", "send error (%d,%d,%d,%s)", 
				client->socket, status, errno, strerror (errno));
		}
		
		cms_client_disconnect (client);
		return;
	}

	if (status == 0)
	{
		cms_ns_info ("libcms", "connection was broken (%d)", client->socket);
		cms_client_disconnect (client);
		return;
	}

	if (status < 0 || status > client->send_data_size)
	{
		cms_ns_error ("libcms", "send failed (%d,%d,%d)", 
			client->socket, status, client->send_data_size);
		client->send_data_size = 0;
		return;
	}

	// data byla odeslana
	client->send_data_size -= status;
	memmove (client->send_data, client->send_data + status, client->send_data_size);
}

int _cms_client_recv_text (cms_client_t * client, char * data, unsigned maxsize)
{
	char * ptr1 = NULL;
	char * ptr2 = NULL;
	int status;
	int size, size1;

	while (!(ptr1 = memchr (client->recv_data, 0x0D, client->recv_data_size))
		&& !(ptr2 = memchr (client->recv_data, 0x0A, client->recv_data_size))
		&& client->recv_data_size < maxsize)
	{
		// precteme data
		status = recv (client->socket, client->recv_data + client->recv_data_size,
			maxsize - client->recv_data_size, MSG_DONTWAIT | MSG_NOSIGNAL);
		//cms_ns_debug ("libcms", "recv %d %d", client->recv_data_size, status);

		if (status == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0)
			{
				return -1;
			}
			
			if (errno == ECONNRESET)
			{
				cms_ns_info ("libcms", "recv error (%d,%d,%d,%d,%s)", 
					client->socket, maxsize, status, errno, strerror (errno));
			}
			else
			{
				cms_ns_error ("libcms", "recv error (%d,%d,%d,%d,%s)", 
					client->socket, maxsize, status, errno, strerror (errno));
			}
			
			cms_client_disconnect (client);
			return -1;
		}

		if (status == 0)
		{
			cms_ns_info ("libcms", "connection was broken (%d)", 
				client->socket);
			cms_client_disconnect (client);
			return -1;
		}

		client->recv_data_size += status;
	}

	// byla prijata zprava ukoncena 0x0D (0x0A)
	if (ptr1)
	{
		size = (long) ptr1 - (long) client->recv_data;
		memcpy (data, client->recv_data, size);
		if (size < maxsize)
		{
			((char *) data)[size] = 0;
		}
		size1 = size + 1;
		if (size1 < client->recv_data_size && ptr1[1] == 0x0A)
		{
			size1++;
		}
		memmove (client->recv_data, client->recv_data + size1, 
			client->recv_data_size - size1);
		client->recv_data_size -= size1;
	}

	// byla prijata zprava ukoncena 0x0A
	else if (ptr2)
	{
		size = (long) ptr2 - (long) client->recv_data;
		memcpy (data, client->recv_data, size);
		if (size < maxsize)
		{
			((char *) data)[size] = 0;
		}
		size1 = size + 1;
		memmove (client->recv_data, client->recv_data + size1, 
			client->recv_data_size - size1);
		client->recv_data_size -= size1;
	}

	// byla prijata zprava maximalni delky
	else
	{
		memcpy (data, client->recv_data, maxsize);
		client->recv_data_size = 0;
		size = maxsize;
	}

	// poznamename si cas
	client->recv_time_count = 0;

	// zachytime pinkaci zpravu
	if (client->ping_type == CMS_PING_TYPE_DATA)
	{
		if (size == 0)
		{
			return -1;
		}
	}

	return size;	
}

void _cms_client_send (cms_client_t * client, const void * data, unsigned size)
{
	if (size >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong size %d", size);
		return;
	}

	// odesleme data
 	switch (client->tcp_mode)
 	{
 		case CMS_TCP_MODE_ELDIS:
 		{
 			_cms_client_send_eldis (client, data, size);
 			break;
 		}
 		case CMS_TCP_MODE_RAW:
 		{
 			_cms_client_send_raw (client, data, size);
 			break;
 		}
 		case CMS_TCP_MODE_TEXT:
 		{
 			_cms_client_send_text (client, data, size);
 			break;
 		}
 	}
}

int _cms_client_recv (cms_client_t * client, void * data, unsigned maxsize)
{
	if (maxsize == 0 || maxsize >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong maxsize %d", maxsize);
		return -1;
	}

	// prijmeme data
	switch (client->tcp_mode)
	{
		case CMS_TCP_MODE_ELDIS:
		{
			return _cms_client_recv_eldis (client, data, maxsize);
		}
		case CMS_TCP_MODE_RAW:
		{
			return _cms_client_recv_raw (client, data, maxsize);
		}
		case CMS_TCP_MODE_TEXT:
		{
			return _cms_client_recv_text (client, data, maxsize);
		}
	}

	return -1;
}

void _cms_client_epoll_add (cms_client_t * client)
{
	struct epoll_event event;
	int status;

	// zamkneme zamek
	g_mutex_lock (client_epoll_mutex);

	// vytvorime epoll
	if (client_epoll_fd == -1)
	{
		client_epoll_fd = epoll_create (1);
		if (client_epoll_fd == -1)
		{
			cms_ns_error ("libcms", "epoll_create error (%d,%s)", 
				errno, strerror (errno));
		}
		status = fcntl (client_epoll_fd, F_SETFD, FD_CLOEXEC);
		if (status == -1)
		{
			cms_ns_error ("libcms", "fcntl F_SETFD FD_CLOEXEC error (%d,%s)", 
				errno, strerror (errno));
		}
		cms_ns_info ("libcms", "epoll_create (%d)", client_epoll_fd);
	}

	// pridame client objekt do seznamu
	client_epoll_list = g_list_append (client_epoll_list, client);

	// povolime signal k prijmu
	client->pollout = FALSE;
	memset (&event, 0, sizeof (event));
	event.events = EPOLLIN;
	event.data.ptr = client;
	status = epoll_ctl (client_epoll_fd, EPOLL_CTL_ADD, client->socket, &event);
	if (status == -1)
	{
		cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_ADD error (%d,%d,%d,%s)", 
			client->socket, client_epoll_fd, errno, strerror (errno));
	}

	// odemkneme zamek
	g_mutex_unlock (client_epoll_mutex);
}

void _cms_client_epoll_remove (cms_client_t * client)
{
	// zamkneme zamek
	g_mutex_lock (client_epoll_mutex);

	// nevyhodime objekt z pollu, ale pockame, az to za nas udela poll thread, 
	// pokud je jiz ukoncene spojeni, melo by k tomu dojit bezprostredne
	while (g_list_find (client_epoll_list, client))
	{
		g_cond_wait (client_epoll_cond, client_epoll_mutex);
	}

	// odemkneme zamek
	g_mutex_unlock (client_epoll_mutex);
}

void __cms_client_epoll_set (cms_client_t * client)
{
	struct epoll_event event;
	int status;

	// jsou data k odeslani
	if (client->send_data_size || !cms_round_buffer_is_empty (client->send_buffer))
	{
		if (!client->pollout)
		{
			// povolime vysilani
			client->pollout = TRUE;
			memset (&event, 0, sizeof (event));
			event.events = EPOLLIN | EPOLLOUT;
			event.data.ptr = client;
			status = epoll_ctl (client_epoll_fd, EPOLL_CTL_MOD, client->socket, &event);
			if (status == -1)
			{
				cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_MOD error (%d,%d,%d,%s)", 
					client->socket, client_epoll_fd, errno, strerror (errno));
			}

			// posleme signal
			g_cond_broadcast (client_epoll_cond);
		}
	}

	// nejsou data k odeslani
	else
	{
		if (client->pollout)
		{
			// zakazeme vysilani
			client->pollout = FALSE;
			memset (&event, 0, sizeof (event));
			event.events = EPOLLIN;
			event.data.ptr = client;
			status = epoll_ctl (client_epoll_fd, EPOLL_CTL_MOD, client->socket, &event);
			if (status == -1)
			{
				cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_MOD error (%d,%d,%d,%s)", 
					client->socket, client_epoll_fd, errno, strerror (errno));
			}

			// posleme signal
			g_cond_broadcast (client_epoll_cond);
		}
	}
}

void _cms_client_epoll_set (cms_client_t * client)
{
	// zamkneme zamek
	g_mutex_lock (client_epoll_mutex);

	// provedeme akci
	__cms_client_epoll_set (client);
	
	// odemkneme zamek
	g_mutex_unlock (client_epoll_mutex);
}

void _cms_client_epoll_in (cms_client_t * client)
{
	char buffer[MSGMAX+4];
	int size, status;

	// prijmeme data
	while ((size = _cms_client_recv (client, buffer, MSGMAX)) >= 0)
	{
		if (client->external_recv_buffer)
		{
			// pokud byl objekt client alokovany z objektu server2
			// a dostal v konstruktoru cislo konekce, pridame toto
			// cislo konekce do dat
			if (client->connection_id)
			{
				memmove (buffer + 4, buffer, size);
				((unsigned char *)buffer)[0] = client->connection_id & 0xff;
				((unsigned char *)buffer)[1] = (client->connection_id >> 8) & 0xff;
				((unsigned char *)buffer)[2] = (client->connection_id >> 16) & 0xff;
				((unsigned char *)buffer)[3] = (client->connection_id >> 24) & 0xff;
				size += 4;
			}
			status = cms_round_buffer_write (client->external_recv_buffer, buffer, size);
			if (status != size)
			{
				cms_ns_error ("libcms", "cms_round_buffer_write error (%d,%d)", 
					size, status);
				cms_client_disconnect (client);
			}
		}
		else if (client->recv_buffer)
		{
			status = cms_round_buffer_write (client->recv_buffer, buffer, size);
			if (status != size)
			{
				cms_ns_error ("libcms", "cms_round_buffer_write error (%d,%d)", 
					size, status);
				cms_client_disconnect (client);
			}
		}
	}
}

void _cms_client_epoll_out (cms_client_t * client)
{
	char buffer[MSGMAX];
	int size;

	// zbyla nejaka data k vysilani od minule
	if (client->send_data_size)
	{
		_cms_client_send (client, NULL, 0);
	}

	// jsou nove data k vysilani v bufferu
	else
	{
		size = cms_round_buffer_read (client->send_buffer, buffer, sizeof (buffer));
		if (size >= 0)
		{
			_cms_client_send (client, buffer, size);
		}
	}

	// pokud nejsou data, zakazeme dalsi vysilani
	_cms_client_epoll_set (client);
}

void _cms_client_epoll_hup (cms_client_t * client)
{
	int status;

	// vymazu buffery
	client->send_data_size = 0;
	client->recv_data_size = 0;

	// ukoncim spojeni
	cms_client_shutdown (client);

	// zamkneme zamek
	g_mutex_lock (client_epoll_mutex);

	// vyhodime objekt ze seznamu
	client_epoll_list = g_list_remove (client_epoll_list, client);

	// vyhodime socket z pollu
	status = epoll_ctl (client_epoll_fd, EPOLL_CTL_DEL, client->socket, NULL);
	if (status == -1)
	{
		cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_DEL error (%d,%d,%d,%s)", 
			client->socket, client_epoll_fd, errno, strerror (errno));
	}
	
	// odemkneme zamek
	g_mutex_unlock (client_epoll_mutex);
	g_cond_broadcast (client_epoll_cond);
}

gpointer cms_client_epoll_thread (gpointer data)
{
	struct epoll_event event;
	cms_client_t * client = NULL;
	int status;

	while (TRUE)
	{
		status = epoll_wait (client_epoll_fd, &event, 1, -1);

		if (status == 1)
		{
			//cms_ns_debug ("libcms", "epoll event");
			client = (cms_client_t *) event.data.ptr;
			if (!client || client->magic != CMS_CLIENT_MAGIC)
			{
				cms_ns_error ("libcms", "wrong magic %08X", client->magic);
			}
			else
			{
				if (event.events & EPOLLIN)
				{
					//cms_ns_debug ("libcms", "EPOLLIN");
					_cms_client_epoll_in (client);
				}
				if (event.events & EPOLLOUT)
				{
					//cms_ns_debug ("libcms", "EPOLLOUT");
					_cms_client_epoll_out (client);
				}
				if (event.events & EPOLLHUP)
				{
					//cms_ns_debug ("libcms", "EPOLLHUP");
					_cms_client_epoll_hup (client);
				}
				if (event.events & EPOLLERR)
				{
					//cms_ns_debug ("libcms", "EPOLLERR");
					shutdown (client->socket, SHUT_RDWR);
				}
			}
		}
		
		else if (status == 0)
		{
			//cms_ns_debug ("libcms", "epoll timeout");
		}

		else if (errno == EINTR)
		{
			// Interrupted system call
		}
		
		else
		{
			cms_ns_error ("libcms", "epoll_wait error (%d,%d,%s)", 
				client_epoll_fd, errno, strerror (errno));
		}
	}

	return NULL;
}

#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

struct icmphdr
{
	unsigned char type;
	unsigned char code;
	unsigned short int checksum;
	union
	{
		struct
		{
			unsigned short int id;
			unsigned short int sequence;
		} echo;
	} un;
};

struct iphdr
{
	unsigned int ihl : 4;
	unsigned int version : 4;
	unsigned char tos;
	unsigned short int tot_len;
	unsigned short int id;
	unsigned short int frag_off;
	unsigned char ttl;
	unsigned char protocol;
	unsigned short int check;
	unsigned long int saddr;
	unsigned long int daddr;
};

void _cms_client_ping ()
{
	struct epoll_event event;
	unsigned char icmpmsg[128];
	struct iphdr * iph;
	struct icmphdr * icmp;
	struct sockaddr * addr;
	socklen_t addrlen;
	char addrbuf[64];
	cms_client_t * client;
	GList * addrlist = NULL;
	GList * addrlink;
	GList * clientlink;
	int pid = -1;
	int status;
	
#ifdef G_OS_UNIX
	// systemove volani gettid
#ifdef __i386__
	pid = syscall (224);
#endif
#ifdef __amd64__
	pid = syscall (186);
#endif
	// v pripade neuspechu pouzijeme getpid
	if (pid == -1)
	{
		pid = getpid ();
	}
#else
	pid = GetCurrentProcessId ();
#endif

	// prijmeme icmp ping
	while (TRUE)
	{
		// prijmeme zpravu
		addr = (struct sockaddr *) addrbuf;
		addrlen = sizeof (addrbuf);
		if (recvfrom (client_ping_fd, icmpmsg, 
			sizeof (icmpmsg), 0, addr, &addrlen) <= 0)
		{
			break;
		}

		// zpracujeme odpoved
		if (addr->sa_family == AF_INET)
		{
			iph = (struct iphdr *) icmpmsg;
			icmp = (struct icmphdr *)(icmpmsg + iph->ihl * 4);

			if (icmp->type == ICMP_ECHOREPLY && ntohs (icmp->un.echo.id) == pid)
			{
				//cms_ns_debug ("libcms", "ping received %d %d", 
				//	ntohs (icmp->un.echo.id), ntohs (icmp->un.echo.sequence));

				for (clientlink = client_epoll_list; clientlink; 
					clientlink = g_list_next (clientlink))
				{
					client = (cms_client_t *) clientlink->data;
					if (client->sockaddr->sa_family == AF_INET)
					{
						if (((struct sockaddr_in *) addr)->sin_addr.s_addr 
							== ((struct sockaddr_in *) client->sockaddr)->sin_addr.s_addr)
						{
							client->recv_time_count = 0;
						}
					}
				}
			}
		}
	}

	// pripravime icmp packet
	client_ping_sequence = (client_ping_sequence + 1) & 0x3ff;
	memset (icmpmsg, 0, sizeof (struct icmphdr));
	icmp = (struct icmphdr *) icmpmsg;
	icmp->type = ICMP_ECHO;
	icmp->un.echo.id = htons (pid);
	icmp->un.echo.sequence = htons (client_ping_sequence);
	icmp->checksum = ipchecksum (icmpmsg, sizeof (struct icmphdr));
	memset (&event, 0, sizeof (event));

	// vysleme ping
	for (clientlink = client_epoll_list; clientlink; 
		clientlink = g_list_next (clientlink))
	{
		client = (cms_client_t *) clientlink->data;
		// lokalni adresa, ping neni potreba
		if (client->loopback)
		{
			continue;
		}
		// client nema nastaveny ping
		if (!client->ping_timeout)
		{
			continue;
		}
		// client nema cas na ping
		if (client_ping_sequence % client->ping_interval)
		{
			continue;
		}

		// odesleme icmp ping
		if (client->ping_type == CMS_PING_TYPE_ICMP
			&& client->sockaddr->sa_family == AF_INET)
		{
			// zjistime, jestli jsme na tuto adresu jiz nepinkali
			for (addrlink = addrlist; addrlink; addrlink = g_list_next (addrlink))
			{
				if (((struct sockaddr_in *) client->sockaddr)->sin_addr.s_addr 
					== ((struct sockaddr_in *) addrlink->data)->sin_addr.s_addr)
				{
					break;
				}
			}

			// pridame adresu do seznamu
			if (!addrlink)
			{
				addrlist = g_list_append (addrlist, client->sockaddr);
			
				// odesleme ping
				status = sendto (client_ping_fd, icmpmsg, sizeof (struct icmphdr), 
					0, client->sockaddr, client->sockaddrlen);
				if (status != sizeof (struct icmphdr))
				{
					cms_ns_error ("libcms", "sento failed (%d,%d,%d,%s)", 
						sizeof (struct icmphdr), status, errno, strerror (errno));
				}
			}
		}

		// odesleme datovy ping
		else if (client->ping_type == CMS_PING_TYPE_DATA)
		{
			// zapiseme data do bufferu
			cms_round_buffer_write (client->send_buffer, NULL, 0);

			// povolime vysilani dat
			__cms_client_epoll_set (client);
		}

		// odesleme stary datovy ping
		else if (client->ping_type == CMS_PING_TYPE_OLD)
		{
			// zapiseme data do bufferu
			char buffer[3];
			memset (buffer, 0, 3);
			cms_round_buffer_write (client->send_buffer, buffer, 3);

			// povolime vysilani dat
			__cms_client_epoll_set (client);
		}
	}

	// smazeme seznam adres
	g_list_free (addrlist);
	
	// prerusime spojeni, kterym vyprsel ping
	for (clientlink = client_epoll_list; clientlink; 
		clientlink = g_list_next (clientlink))
	{
		client = (cms_client_t *) clientlink->data;
		// lokalni adresa, ping neni potreba
		if (client->loopback)
		{
			continue;
		}
		// client nema nastaveny ping
		if (!client->ping_timeout)
		{
			continue;
		}

		if (client->recv_time_count++ > client->ping_timeout)
		{
			cms_ns_warning ("libcms", "ping timeout (%d) %s:%x:%d %d %d", 
				client->socket, client->remote_addr, client->remote_port, 
				client->remote_port, client->recv_time_count, client->ping_timeout);
			cms_client_disconnect (client);
		}
	}
}

gpointer cms_client_ping_thread (gpointer data)
{
	int status;
	
	// alokujeme socket
	client_ping_fd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (client_ping_fd == -1)
	{
		cms_ns_error ("libcms", "socket error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}

#ifdef G_OS_UNIX
	// nastavime parametry socketu
	status = fcntl (client_ping_fd, F_SETFD, FD_CLOEXEC);
	if (status == -1)
	{
		cms_ns_error ("libcms", "fcntl F_SETFD error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}
#endif

#ifdef G_OS_UNIX
	status = fcntl (client_ping_fd, F_SETFL, O_NONBLOCK);
	if (status == -1)
	{
		cms_ns_error ("libcms", "fcntl F_SETFL error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}
#else
	unsigned long arg = 1;
	status = ioctlsocket (client_ping_fd, FIONBIO, &arg);
	if (status == SOCKET_ERROR)
	{
		cms_ns_error ("libcms", "ioctlsocket FIONBIO error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}
#endif

	// nastavime time to live
	unsigned char ttl = 255;
	status = setsockopt (client_ping_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof (ttl));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt error (%d,%s)", 
			errno, strerror (errno));
		return NULL;
	}

	while (TRUE)
	{
		// zamkneme zamek
		g_mutex_lock (client_epoll_mutex);

		// volame ping
		_cms_client_ping ();
		
		// odemkneme zamek
		g_mutex_unlock (client_epoll_mutex);

		// cekame desetinu vteriny
		g_usleep (100000);
	}

	return NULL;
}

