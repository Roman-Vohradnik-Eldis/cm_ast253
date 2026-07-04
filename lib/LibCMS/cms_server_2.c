#include "cms_server_2.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_SERVER2_MAGIC 0x72060908

gpointer cms_server_2_new_thread (gpointer data);
gpointer cms_server_2_delete_thread (gpointer data);
extern void _cms_client_init ();

cms_server_2_t * cms_server_2_new (
	int send_recv_flags, 
	const char * addr, 
	const char * port,
	unsigned ping,
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size,
	gboolean (* on_client_new) (cms_client_t *, void *),
	void *on_client_new_data,
	gboolean (* on_client_delete) (cms_client_t *, void *),
	void *on_client_delete_data,
	cms_round_buffer_t * recv_buffer)
{
	char text[1024];
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
	cms_server_2_t * server = (cms_server_2_t *) g_malloc0 (sizeof (cms_server_2_t));
	if (!server)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujme promenne
	server->run = TRUE;
	server->init = FALSE;
	server->socket = -1;
	server->port = g_strdup (port);
	server->send_recv_flags = send_recv_flags;
	server->ping = ping;
	server->tcp_mode = mode;
	server->max_buffer_size = max_buffer_size;
	server->on_cms_client_new = on_client_new;
	server->on_client_new_data = on_client_new_data;
	server->on_cms_client_delete = on_client_delete;
	server->on_client_delete_data = on_client_delete_data;

	// vytvorime zamek
	server->mutex = g_mutex_new ();
	if (!server->mutex)
	{
		cms_ns_error ("libcms", "g_mutex_new error");
		goto failed;
	}

	// vytvorime prijimaci buffer
	if (send_recv_flags & CMS_FLAG_RECV)
	{
		if (recv_buffer)
		{
			server->external_recv_buffer = recv_buffer;
		}
		else
		{
			g_snprintf (text, sizeof (text), "server receive buffer %s:%s", addr, port);
			server->recv_buffer = cms_round_buffer_new_with_block (text, 
				server->max_buffer_size, CMS_ROUND_BUFFER_READ_BLOCK 
				| CMS_ROUND_BUFFER_WRITE_GROW);
			if (!server->recv_buffer)
			{
				cms_ns_error ("libcms", "cms_round_buffer_new error");
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
	status = getaddrinfo (addr, tmp_port, &hints, &server->addrinfo);
	if (status != 0)
	{
		cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d)", 
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

	// nastavime magicke cislo a vratime objekt
	server->magic = CMS_SERVER2_MAGIC;
	return server;

failed:
	if (server)
	{
		if (server->socket != -1)
		{
			close (server->socket);
		}
		if (server->port)
		{
			g_free (server->port);
		}
		if (server->addrinfo)
		{
			freeaddrinfo (server->addrinfo);
		}
		if (server->recv_buffer)
		{
			cms_round_buffer_delete (server->recv_buffer);
		}
		if (server->mutex)
		{
			g_mutex_free (server->mutex);
		}
		g_free (server);
	}

	return NULL;
}

void cms_server_2_delete (cms_server_2_t * server)
{
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return;
	}

	// ukoncime cinnost
	cms_server_2_shutdown (server);

	// smazeme magicke cislo
	server->magic = 0;

	// uvolnime polozky objektu
	if (server->socket != -1)
	{
		close (server->socket);
	}
	if (server->port)
	{
		g_free (server->port);
	}
	if (server->addrinfo)
	{
		freeaddrinfo (server->addrinfo);
	}
	if (server->recv_buffer)
	{
		cms_round_buffer_delete (server->recv_buffer);
	}
	if (server->mutex)
	{
		g_mutex_free (server->mutex);
	}

	// uvolnime objekt
	g_free (server);
}

void cms_server_2_init (cms_server_2_t * server)
{
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return;
	}

	// inicializace uz probehla
	if (server->init)
	{
		return;
	}

	// zamkneme zamek
	g_mutex_lock (server->mutex);
	
	// poznamename, ze inicializace je dokoncena
	server->init = TRUE;
	
	// nastartujeme thread na prijimani novych spojeni
	if (!server->server_new_thread)
	{
		server->server_new_thread = g_thread_create_full (
			cms_server_2_new_thread, server, 0x100000, 
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);
		if (!server->server_new_thread)
		{
			cms_ns_error ("libcms", "g_thread_create_full error");
		}
	}

	// nastartujeme thread na ruseni starych spojeni
	if (!server->server_delete_thread)
	{
		server->server_delete_thread = g_thread_create_full (
			cms_server_2_delete_thread, server, 0x100000,
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);
		if (!server->server_delete_thread)
		{
			cms_ns_error ("libcms", "g_thread_create_full error");
		}
	}

	// zamkneme zamek
	g_mutex_unlock (server->mutex);
}

void cms_server_2_shutdown (cms_server_2_t * server)
{
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return;
	}

	// dame pokyn k ukonceni cinnosti
	server->run = FALSE;

	// ukoncime cinnost socketu
	if (server->socket != -1)
	{
		shutdown (server->socket, SHUT_RDWR);
	}

	// ukoncime cinnost prijimaciho buffer
	if (server->recv_buffer)
	{
		cms_round_buffer_set_flags (server->recv_buffer, 0);
		cms_round_buffer_clear (server->recv_buffer);
	}
	// pockame na ukonceni threadu
	if (server->server_new_thread)
	{
		g_thread_join (server->server_new_thread);
		server->server_new_thread = NULL;
	}
	if (server->server_delete_thread)
	{
		g_thread_join (server->server_delete_thread);
		server->server_delete_thread = NULL;
	}
}

void cms_server_2_disconnect (cms_server_2_t * server, unsigned connection_id)
{
	GList * link;

	g_mutex_lock (server->mutex);

	for (link = server->client_list; link; link = g_list_next (link))
	{
		cms_client_t * client = (cms_client_t *) link->data;
		if (client->connection_id == connection_id)
		{
			cms_client_disconnect (client);
			break;
		}
	}

	g_mutex_unlock (server->mutex);
	
}

gboolean cms_server_2_is_connected (cms_server_2_t * server)
{
	gboolean connected = FALSE;
	GList * link;

	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return FALSE;
	}

	g_mutex_lock (server->mutex);

	for (link = server->client_list; link; link = g_list_next (link))
	{
		cms_client_t * client = (cms_client_t *) link->data;
		if (cms_client_is_connected (client))
		{
			connected = TRUE;
		}
	}

	g_mutex_unlock (server->mutex);

	return connected;
}

gchar * cms_server_2_get_remote_addr (cms_server_2_t * server, unsigned connection_id)
{
	GString * string;
	GList * link;
	
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return 0;
	}

	g_mutex_lock (server->mutex);

	string = g_string_new ("");

	for (link = server->client_list; link; link = g_list_next (link))
	{
		cms_client_t * client = (cms_client_t *) link->data;
		if (cms_client_is_connected (client)
			&& (connection_id == 0
			|| connection_id == client->connection_id))
		{
			if (string->len)
			{
				g_string_append (string, " ");
			}
			g_string_append (string, cms_client_get_remote_addr (client));
		}
	}

	g_mutex_unlock (server->mutex);

	return g_string_free (string, FALSE);
}

int cms_server_2_send (cms_server_2_t * server, const void * buffer, 
	unsigned size, unsigned connection_id)
{
	GList * link;

	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return 0;
	}

	if (size >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong size: %d", size);
		return -1;
	}

	// objekt neni inicializovany na vysilani
	if (!(server->send_recv_flags & CMS_FLAG_SEND))
	{
		cms_ns_error ("libcms", "object is not initialized for sending");
		return -1;
	}
	
	// pokud je treba, inicializujeme objekt
	cms_server_2_init (server);
	
	// cinnost objektu jiz byla ukoncena
	if (!server->run)
	{
		return -1;
	}

	g_mutex_lock (server->mutex);

	for (link = server->client_list; link; link = g_list_next (link))
	{
		cms_client_t * client = (cms_client_t *) link->data;

		// konekce je aktivni, posleme data
		if (cms_client_is_connected (client)
			&& (connection_id == 0
			|| connection_id == client->connection_id))
		{
			cms_client_send (client, buffer, size);
		}
	}

	g_mutex_unlock (server->mutex);

	return size;
}

int cms_server_2_recv (cms_server_2_t * server, void * data, 
	unsigned maxsize, unsigned * connection_id)
{
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return -1;
	}

	if (maxsize == 0 || maxsize >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong maxsize: %d", maxsize);
		return -1;
	}

	// objekt neni inicializovan na prijem
	if (!(server->send_recv_flags & CMS_FLAG_RECV))
	{
		cms_ns_error ("libcms", "object is not initialized for receiving");
		return -1;
	}

	// objekt neni inicializovan na prime cteni
	if (server->external_recv_buffer)
	{
		return -1;
	}

	// pokud je treba, inicializujeme objekt
	cms_server_2_init (server);

	// cinnost objektu jiz byla ukoncena
	if (!server->run)
	{
		return -1;
	}

	if (server->tcp_mode == CMS_TCP_MODE_TEXT)
	{
		if (maxsize > 0)
		{
			maxsize--;
		}
	}
	
	unsigned char buffer[maxsize + 4];
	unsigned id;
	int size;
	
	size = cms_round_buffer_read (server->recv_buffer, buffer, maxsize + 4);

	// pri ShutDown
	if (size < 4)
	{
		if (connection_id)
		{
			*connection_id = 0;
		}
		return -1;

	}

	// normalni provoz
	else
	{
		// vycteme cislo konekce a vyhodime ho z dat
		id = buffer[0];
		id |= ((unsigned) buffer[1]) << 8;
		id |= ((unsigned) buffer[2]) << 16;
		id |= ((unsigned) buffer[3]) << 24;
		if (connection_id)
		{
			*connection_id = id;
		}
		size -= 4;
		memcpy (data, buffer + 4, size);
	}

	// prijimame v textovem rezimu, zakoncime nulou
	if (server->tcp_mode == CMS_TCP_MODE_TEXT)
	{
		if (size >= 0 && maxsize > 0)
		{
			((char *)data)[size] = 0;
		}
	}

	return size;
}

gpointer cms_server_2_new_thread (gpointer data)
{
	cms_server_2_t * server = (cms_server_2_t *) data;
	cms_client_t * client;
	GList * link;
	int socket;
	int status;
	int one;
	
	while (server->run)
	{
		// prijmeme pripojeni noveho klienta
		socket = accept (server->socket, NULL, NULL);
		if (socket == -1)
		{
			cms_ns_warning ("libcms", "accept error (%d,%s)", 
				errno, strerror (errno));
			continue;
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

		// zavreme pristup k seznamu spojeni
		g_mutex_lock (server->mutex);

again:
		// najdeme unikatni connection id
		if (++server->last_connection_id == 0)
		{
			server->last_connection_id = 1;
		}
		for (link = server->client_list; link; link = g_list_next (link))
		{
			cms_client_t * client = (cms_client_t *) link->data;
			if (server->last_connection_id == client->connection_id)
			{
				goto again;
			}
		}

		// otevreme pristup k seznamu spojeni
		g_mutex_unlock (server->mutex);

		// vytvorime spojeni na klienta
		client = cms_client_new_from_server (server->send_recv_flags, socket, 
			server->ping, server->tcp_mode, server->max_buffer_size,
			server->external_recv_buffer ? server->external_recv_buffer : server->recv_buffer,
			server->external_recv_buffer ? 0 : server->last_connection_id);
		if (!client)
		{
			cms_ns_error ("libcms", "cms_client_new_from_server error");
			close (socket);
			continue;
		}
		
		// inicializujeme objekt
		cms_client_init (client);

		// zavreme pristup k seznamu spojeni
		g_mutex_lock (server->mutex);

		// provedeme akci pro noveho klienta
		if (server->on_cms_client_new)
		{
			while (!server->on_cms_client_new (client, server->on_client_new_data))
			{
				g_mutex_unlock (server->mutex);
				g_thread_yield ();
				g_mutex_lock (server->mutex);
			}		
		}

		// pridam spojeni do seznamu
		server->client_list = g_list_append (server->client_list, client);

		// otevreme pristup k seznamu spojeni
		g_mutex_unlock (server->mutex);
	}

	// zamkneme pristup k seznamu
	g_mutex_lock (server->mutex);

	// ukoncime vsechna spojeni
	for (link = server->client_list; link; link = g_list_next (link))
	{
		cms_client_t * client = (cms_client_t *) link->data;
		
		// prerusime spojeni a zmenime flagy vysilaciho bufferu
		cms_client_disconnect (client);

		// provedeme akci pro rusiciho se klienta
		if (server->on_cms_client_delete)
		{
			while (!server->on_cms_client_delete (client, server->on_client_delete_data))
			{
				g_mutex_unlock (server->mutex);
				g_thread_yield ();
				g_mutex_lock (server->mutex);	
			}
		}

		cms_client_delete (client);
	}

	g_list_free (server->client_list);
	server->client_list = NULL;

	// odemkneme pristup
	g_mutex_unlock (server->mutex);

	return NULL;
}

gpointer cms_server_2_delete_thread (gpointer data)
{
	cms_server_2_t * server = (cms_server_2_t *) data;
	GList * link;

	while (server->run)
	{
		// zamkneme pristup k seznamu spojeni
		g_mutex_lock (server->mutex);

again:
		// vyradime vsechna neaktivni spojeni
		for (link = server->client_list; link; link = g_list_next (link))
		{
			cms_client_t * client = (cms_client_t *) link->data;
			if (!cms_client_is_connected (client))
			{
				server->client_list = g_list_delete_link (server->client_list, link);
				// provedeme akci pro rusiciho se klienta
				if (server->on_cms_client_delete)
				{
					while (!server->on_cms_client_delete (client, server->on_client_delete_data))
					{
						g_mutex_unlock (server->mutex);
						g_thread_yield ();
						g_mutex_lock (server->mutex);	
					}
				}
				cms_client_delete (client);
				goto again;
			}
		}

		// odemkneme pristup k seznamu spojeni
		g_mutex_unlock (server->mutex);

		g_usleep (100000);
	}

	return NULL;
}

unsigned cms_server_2_get_recv_buffer_fill_space (cms_server_2_t * server)
{
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return -1;
	}
	return cms_round_buffer_get_fill_space (server->recv_buffer);
}

unsigned cms_server_2_get_recv_buffer_messages (cms_server_2_t * server)
{
	if (!server || server->magic != CMS_SERVER2_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", server->magic);
		return -1;
	}
	return cms_round_buffer_get_messages (server->recv_buffer);
}
