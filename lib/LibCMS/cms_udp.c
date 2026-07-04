#include "cms_debug.h"
#include "cms_udp.h"
#include "cms_misc.h"

#define CMS_UDP_MAGIC 0x7206090A

void _cms_udp_epoll_add (cms_udp_t * udp);
void _cms_udp_epoll_remove (cms_udp_t * udp);
void _cms_udp_epoll_set (cms_udp_t * udp);
gpointer cms_udp_epoll_thread (gpointer data);

static GMutex * udp_epoll_mutex = NULL;
static GCond * udp_epoll_cond = NULL;
static GThread * udp_epoll_thread = NULL;
static GList * udp_epoll_list = NULL;
static int udp_epoll_fd = -1;

void _cms_udp_init ()
{
	if (!udp_epoll_mutex)
	{
		udp_epoll_mutex = g_mutex_new ();
	}
	if (!udp_epoll_cond)
	{
		udp_epoll_cond = g_cond_new ();
	}
}

cms_udp_t * cms_udp_new (
	int send_recv_flags,
	const char * recv_addr, 
	const char * recv_port, 
	const char * send_addr, 
	const char * send_port,
	const char ** source_addr,
	cms_round_buffer_t * recv_buffer)
{
	struct addrinfo hints;
	struct addrinfo * addrinfo;
	char tmp_send_port[16];
	char tmp_recv_port[16];
	char * ptr;
	int status;
	int i;

#ifdef G_OS_WIN32
	WSADATA data;
	WSAStartup (0x0101, &data);
#endif

	// inicializujeme globalni udp zamky
	_cms_udp_init ();

	// alokujeme objekt
	cms_udp_t * udp = (cms_udp_t *) g_malloc0 (sizeof (cms_udp_t));
	if (!udp)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujeme promenne
	udp->socket = -1;
	udp->recv_port = g_strdup (recv_port);
	udp->send_port = g_strdup (send_port);
	udp->send_recv_flags = send_recv_flags;

	// vytvorime buffer na vysilani
	if (send_recv_flags & CMS_FLAG_SEND)
	{
		udp->send_buffer = cms_round_buffer_new_with_block ("udp send", 
			CMS_UDP_MAX_BUFFER_SIZE, CMS_ROUND_BUFFER_WRITE_GROW 
			| CMS_ROUND_BUFFER_WRITE_THROW);
		if (!udp->send_buffer)
		{
			goto failed;
		}
	}

	// vytvorime buffer na prijem
	if (send_recv_flags & CMS_FLAG_RECV)
	{
		if (recv_buffer)
		{
			udp->external_recv_buffer = recv_buffer;
		}
		else
		{
			udp->recv_buffer = cms_round_buffer_new_with_block ("udp recv", 
				CMS_UDP_MAX_BUFFER_SIZE, CMS_ROUND_BUFFER_READ_BLOCK 
				| CMS_ROUND_BUFFER_WRITE_GROW | CMS_ROUND_BUFFER_WRITE_THROW);
			if (!udp->recv_buffer)
			{
				goto failed;
			}
		}
	}

	// nastavime adresu pro vysilani dat
	if (send_addr && send_addr[0] 
		&& send_port && send_port)
	{
		// upravime cislo portu, pokud je zadano hexadecimalne
		if (send_port[0] == '0' && send_port[1] == 'x')
		{
			status = strtol (send_port + 2, &ptr, 16);
			if (!ptr || *ptr)
			{
				cms_ns_error ("libcms", "wrong port number %s", send_port);
				goto failed;
			}
			sprintf (tmp_send_port, "%d", status);
		}
		else
		{
			strcpy (tmp_send_port, send_port);
		}
		
		// nastavime vysilaci adresu
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		status = getaddrinfo (send_addr, tmp_send_port, &hints, &udp->send_addrinfo);
		if (status != 0)
		{
			cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
				send_addr, send_port, status, errno, strerror (errno));
			goto failed;
		}

		// vysilaci adresa je multicastova
		if (udp->send_addrinfo->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in * addr_in = (struct sockaddr_in *) udp->send_addrinfo->ai_addr;
			if (IN_MULTICAST (ntohl (addr_in->sin_addr.s_addr)))
			{
				cms_ns_error ("libcms", "%s is multicast address", send_addr);
				goto failed;
			}
		}
		else if (udp->send_addrinfo->ai_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) udp->send_addrinfo->ai_addr;
			if (IN6_IS_ADDR_MULTICAST (addr_in6->sin6_addr.s6_addr))
			{
				cms_ns_error ("libcms", "%s is multicast address", send_addr);
				goto failed;
			}
		}
	}
	
	// nastavime adresu pro prijem dat
	if (recv_addr && recv_addr[0] 
		&& recv_port && recv_port[0])
	{
		// upravime cislo portu, pokud je zadano hexadecimalne
		if (recv_port[0] == '0' && recv_port[1] == 'x')
		{
			status = strtol (recv_port + 2, &ptr, 16);
			if (!ptr || *ptr)
			{
				cms_ns_error ("libcms", "wrong port number %s", recv_port);
				goto failed;
			}
			sprintf (tmp_recv_port, "%d", status);
		}
		else
		{
			strcpy (tmp_recv_port, recv_port);
		}
		
		// nastavime prijimaci adresu
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		status = getaddrinfo (recv_addr, tmp_recv_port, &hints, &udp->recv_addrinfo);
		if (status != 0)
		{
			cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
				recv_addr, recv_port, status, errno, strerror (errno));
			goto failed;
		}

		// prijimaci adresa je multicastova
		if (udp->recv_addrinfo->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in * addr_in = (struct sockaddr_in *) udp->recv_addrinfo->ai_addr;
			if (IN_MULTICAST (ntohl (addr_in->sin_addr.s_addr)))
			{
				cms_ns_error ("libcms", "%s is multicast address", recv_addr);
				goto failed;
			}
		}
		else if (udp->recv_addrinfo->ai_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) udp->recv_addrinfo->ai_addr;
			if (IN6_IS_ADDR_MULTICAST (addr_in6->sin6_addr.s6_addr))
			{
				cms_ns_error ("libcms", "%s is multicast address", recv_addr);
				goto failed;
			}
		}

		// nastavime adresu, ze ktere musi byt vysilana prijimana data
		if (source_addr)
		{
			memset (&hints, 0, sizeof (hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;

			for (i = 0; source_addr[i]; i++)
			{
				status = getaddrinfo (source_addr[i], NULL, &hints, &addrinfo);
				if (status != 0)
				{
					cms_ns_error ("libcms", "getaddrinfo error (%s,%d,%d,%s)", 
						source_addr[i], status, errno, strerror (errno));
					goto failed;
				}
				udp->source_addrinfo = g_list_append (udp->source_addrinfo, addrinfo);
			}
		}
	}

	// otevreme socket
	if (udp->recv_addrinfo)
	{
		udp->socket = socket (udp->recv_addrinfo->ai_family,
			udp->recv_addrinfo->ai_socktype, udp->recv_addrinfo->ai_protocol);
	}
	else if (udp->send_addrinfo)
	{
		udp->socket = socket (udp->send_addrinfo->ai_family,
			udp->send_addrinfo->ai_socktype, udp->send_addrinfo->ai_protocol);
	}
	if (udp->socket == -1)
	{
		cms_ns_error ("libcms", "socket error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	// zablokujeme dedeni otevreneho descriptoru do potomku procesu
#ifdef G_OS_UNIX
	status = fcntl (udp->socket, F_SETFD, FD_CLOEXEC);
	if (status == -1)
	{
		cms_ns_error ("libcms", "fcntl F_SETFD error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}
#endif
	
	// povolime broadcast
	int broadcast = 1;
	status = setsockopt (udp->socket, SOL_SOCKET, 
		SO_BROADCAST, &broadcast, sizeof (broadcast));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SOL_SOCKET SO_BROADCAST error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}
	
	// povolime znovupouziti portu po restartu serveru
	int reuse = 1;
	status = setsockopt (udp->socket, SOL_SOCKET, 
		SO_REUSEADDR, &reuse, sizeof (reuse));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SOL_SOCKET SO_REUSEADDR error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	if (udp->recv_addrinfo)
	{
		// navazeme socket na lokalni interface
		status = bind (udp->socket, udp->recv_addrinfo->ai_addr, 
			udp->recv_addrinfo->ai_addrlen);
		if (status == -1)
		{
			cms_ns_error ("libcms", "bind error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}
	}

	// nastavime magicke cislo
	udp->magic = CMS_UDP_MAGIC;
	
	// pridame objekt do epollu
	_cms_udp_epoll_add (udp);

	// vratime objekt
	return udp;

failed:
	if (udp)
	{
		if (udp->socket != -1)
		{
			close (udp->socket);
		}
		if (udp->recv_port)
		{
			g_free (udp->recv_port);
		}
		if (udp->send_port)
		{
			g_free (udp->send_port);
		}
		if (udp->send_addrinfo)
		{
			freeaddrinfo (udp->send_addrinfo);
		}
		if (udp->recv_addrinfo)
		{
			freeaddrinfo (udp->recv_addrinfo);
		}
		while (udp->source_addrinfo)
		{
			freeaddrinfo ((struct addrinfo *)udp->source_addrinfo->data);
			udp->source_addrinfo = g_list_delete_link (udp->source_addrinfo, udp->source_addrinfo);
		}
		if (udp->send_buffer)
		{
			cms_round_buffer_delete (udp->send_buffer);
		}
		if (udp->recv_buffer)
		{
			cms_round_buffer_delete (udp->recv_buffer);
		}
		g_free (udp);
	}
	
	return NULL;
}

cms_udp_t * cms_udp_new_multicast (
	int send_recv_flags, 
	const char * recv_multicast_addr, 
	const char * recv_local_addr,
	const char * recv_port, 
	const char * send_multicast_addr,
	const char * send_local_addr, 
	const char * send_port,
	cms_round_buffer_t * recv_buffer)
{
	struct addrinfo * send_addrinfo = NULL;
	struct addrinfo * recv_addrinfo = NULL;
	struct addrinfo hints;
	char tmp_port[16];
	char * ptr;
	int status;
	
#ifdef G_OS_WIN32
	WSADATA data;
	WSAStartup (0x0101, &data);
#endif

	// inicializujeme globalni udp zamky
	_cms_udp_init ();

	// alokujeme udp objekt
	cms_udp_t * udp = (cms_udp_t *) g_malloc0 (sizeof (cms_udp_t));
	if (!udp)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujeme promenne
	udp->socket = -1;
	udp->recv_port = g_strdup (recv_port);
	udp->send_port = g_strdup (send_port);
	udp->send_recv_flags = send_recv_flags;

	// vytvorime buffer na vysilani
	if (send_recv_flags & CMS_FLAG_SEND)
	{
		udp->send_buffer = cms_round_buffer_new_with_block ("udp send", 
			CMS_UDP_MAX_BUFFER_SIZE, CMS_ROUND_BUFFER_WRITE_GROW 
			| CMS_ROUND_BUFFER_WRITE_THROW);
		if (!udp->send_buffer)
		{
			goto failed;
		}
	}

	// vytvorime buffer na prijem
	if (send_recv_flags & CMS_FLAG_RECV)
	{
		if (recv_buffer)
		{
			udp->external_recv_buffer = recv_buffer;
		}
		else
		{
			udp->recv_buffer = cms_round_buffer_new_with_block ("udp recv", 
				CMS_UDP_MAX_BUFFER_SIZE, CMS_ROUND_BUFFER_READ_BLOCK 
				| CMS_ROUND_BUFFER_WRITE_GROW | CMS_ROUND_BUFFER_WRITE_THROW);
			if (!udp->recv_buffer)
			{
				goto failed;
			}
		}
	}

	// nastavime adresu pro vysilani dat
	if (send_multicast_addr && send_multicast_addr[0]
		&& send_local_addr && send_local_addr[0] 
		&& send_port && send_port[0])
	{
		// upravime cislo portu, pokud je zadano hexadecimalne
		if (send_port[0] == '0' && send_port[1] == 'x')
		{
			status = strtol (send_port + 2, &ptr, 16);
			if (!ptr || *ptr)
			{
				cms_ns_error ("libcms", "wrong port number %s", send_port);
				goto failed;
			}
			sprintf (tmp_port, "%d", status);
		}
		else
		{
			strcpy (tmp_port, send_port);
		}
		
		// nastavime multicastovou adresu pro vysilani
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		status = getaddrinfo (send_multicast_addr, tmp_port, &hints, &udp->send_addrinfo);
		if (status != 0)
		{
			cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
				send_multicast_addr, send_port, status, errno, strerror (errno));
			goto failed;
		}

		// adresa neni multicastova
		if (udp->send_addrinfo->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in * addr_in = (struct sockaddr_in *) udp->send_addrinfo->ai_addr;
			if (!IN_MULTICAST (ntohl (addr_in->sin_addr.s_addr)))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					send_multicast_addr);
				goto failed;
			}
		}
		else if (udp->send_addrinfo->ai_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) udp->send_addrinfo->ai_addr;
			if (!IN6_IS_ADDR_MULTICAST (addr_in6->sin6_addr.s6_addr))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					send_multicast_addr);
				goto failed;
			}
		}

		// nastavime lokalni adresu pro vysilani
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		status = getaddrinfo (send_local_addr, tmp_port, &hints, &send_addrinfo);
		if (status != 0)
		{
			cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
				send_local_addr, send_port, status, errno, strerror (errno));
			goto failed;
		}

		// adresa je multicastova
		if (send_addrinfo->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in * addr_in = (struct sockaddr_in *) send_addrinfo->ai_addr;
			if (IN_MULTICAST (ntohl (addr_in->sin_addr.s_addr)))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					send_multicast_addr);
				goto failed;
			}
		}
		else if (send_addrinfo->ai_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) send_addrinfo->ai_addr;
			if (IN6_IS_ADDR_MULTICAST (addr_in6->sin6_addr.s6_addr))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					send_multicast_addr);
				goto failed;
			}
		}

		// adresy nejsou ze stejne rodiny
		if (udp->send_addrinfo->ai_family != send_addrinfo->ai_family)
		{
			cms_ns_error ("libcms", "wrong address family (%d,%d)", 
				udp->send_addrinfo->ai_family, 
				send_addrinfo->ai_family);
		}	
	}
	
	// nastavime adresu pro prijem dat
	if (recv_multicast_addr && recv_multicast_addr[0]
		&& recv_local_addr && recv_local_addr[0] 
		&& recv_port && recv_port[0])
	{
		// upravime cislo portu, pokud je zadano hexadecimalne
		if (recv_port[0] == '0' && recv_port[1] == 'x')
		{
			status = strtol (recv_port + 2, &ptr, 16);
			if (!ptr || *ptr)
			{
				cms_ns_error ("libcms", "wrong port number %s", recv_port);
				goto failed;
			}
			sprintf (tmp_port, "%d", status);
		}
		else
		{
			strcpy (tmp_port, recv_port);
		}
		
		// nastavime multicastovou adresu pro prijem
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		status = getaddrinfo (recv_multicast_addr, tmp_port, &hints, &udp->recv_addrinfo);
		if (status != 0)
		{
			cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
				recv_multicast_addr, recv_port, status, errno, strerror (errno));
			goto failed;
		}

		// adresa neni multicastova
		if (udp->recv_addrinfo->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in * addr_in = (struct sockaddr_in *) udp->recv_addrinfo->ai_addr;
			if (!IN_MULTICAST (ntohl (addr_in->sin_addr.s_addr)))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					recv_multicast_addr);
				goto failed;
			}
		}
		else if (udp->recv_addrinfo->ai_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) udp->recv_addrinfo->ai_addr;
			if (!IN6_IS_ADDR_MULTICAST (addr_in6->sin6_addr.s6_addr))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					recv_multicast_addr);
				goto failed;
			}
		}

		// nastavime lokalni adresu pro prijem
		memset (&hints, 0, sizeof (hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		status = getaddrinfo (recv_local_addr, tmp_port, &hints, &recv_addrinfo);
		if (status != 0)
		{
			cms_ns_error ("libcms", "getaddrinfo error (%s,%s,%d,%d,%s)", 
				recv_multicast_addr, recv_port, status, errno, strerror (errno));
			goto failed;
		}

		// adresa je multicastova
		if (recv_addrinfo->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in * addr_in = (struct sockaddr_in *) recv_addrinfo->ai_addr;
			if (IN_MULTICAST (ntohl (addr_in->sin_addr.s_addr)))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					recv_multicast_addr);
				goto failed;
			}
		}
		else if (recv_addrinfo->ai_addr->sa_family == AF_INET6)
		{
			struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *) recv_addrinfo->ai_addr;
			if (IN6_IS_ADDR_MULTICAST (addr_in6->sin6_addr.s6_addr))
			{
				cms_ns_error ("libcms", "%s is not multicast address", 
					recv_multicast_addr);
				goto failed;
			}
		}

		// adresy nejsou ze stejne rodiny
		if (udp->recv_addrinfo->ai_family != recv_addrinfo->ai_family)
		{
			cms_ns_error ("libcms", "wrong address family (%d,%d)", 
				udp->recv_addrinfo->ai_family, recv_addrinfo->ai_family);
		}	
	}

	// otevreme socket
	if (udp->recv_addrinfo)
	{
		udp->socket = socket (udp->recv_addrinfo->ai_family,
			udp->recv_addrinfo->ai_socktype, udp->recv_addrinfo->ai_protocol);
	}
	else if (udp->send_addrinfo)
	{
		udp->socket = socket (udp->send_addrinfo->ai_family,
			udp->send_addrinfo->ai_socktype, udp->send_addrinfo->ai_protocol);
	}
	if (udp->socket == -1)
	{
		cms_ns_error ("libcms", "socket error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	// zablokujeme dedeni otevreneho descriptoru do potomku procesu
#ifdef G_OS_UNIX
	status = fcntl (udp->socket, F_SETFD, FD_CLOEXEC);
	if (status == -1)
	{
		cms_ns_error ("libcms", "fcntl F_SETFD error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}
#endif
	
	// povolime broadcast
	int broadcast = 1;
	status = setsockopt (udp->socket, SOL_SOCKET, 
		SO_BROADCAST, &broadcast, sizeof (broadcast));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SOL_SOCKET SO_BROADCAST error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}
	
	// povolime znovupouziti portu po restartu serveru
	int reuse = 1;
	status = setsockopt (udp->socket, SOL_SOCKET, 
		SO_REUSEADDR, &reuse, sizeof (reuse));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt SOL_SOCKET SO_REUSEADDR error (%d,%s)", 
			errno, strerror (errno));
		goto failed;
	}

	if (udp->send_addrinfo)
	{
		// nastavime interface, pres ktery se bude vysilat
		status = setsockopt (udp->socket, IPPROTO_IP, IP_MULTICAST_IF, 
			send_addrinfo->ai_addr, send_addrinfo->ai_addrlen);
		if (status == -1)
		{
			cms_ns_error ("libcms", "setsockopt IPPROTO_IP IP_MULTICAST_IF error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}

		// pocitac bude data posilat i sam sobe
		unsigned char loop = 1;
		status = setsockopt (udp->socket, IPPROTO_IP, 
			IP_MULTICAST_LOOP, &loop, sizeof (loop));
		if (status == -1)
		{
			cms_ns_error ("libcms", "setsockopt IPPROTO_IP IP_MULTICAST_LOOP error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}

		// nastavime time-to-live
		unsigned char ttl = 32;
		status = setsockopt (udp->socket, IPPROTO_IP, 
			IP_MULTICAST_TTL, &ttl, sizeof (ttl));
		if (status == -1)
		{
			cms_ns_error ("libcms", "setsockopt IPPROTO_IP IP_MULTICAST_TTL error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}

		freeaddrinfo (send_addrinfo);
		send_addrinfo = NULL;
	}

	if (udp->recv_addrinfo)
	{
		if (recv_addrinfo->ai_family == AF_INET)
		{
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = ((struct sockaddr_in *) 
				udp->recv_addrinfo->ai_addr)->sin_addr.s_addr;
			mreq.imr_interface.s_addr = ((struct sockaddr_in *) 
				recv_addrinfo->ai_addr)->sin_addr.s_addr;
			status = setsockopt (udp->socket, IPPROTO_IP, 
				IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq));
			if (status == -1)
			{
				cms_ns_error ("libcms", "setsockopt IPPROTO_IP IP_ADD_MEMBERSHIP error (%d,%s)", 
					errno, strerror (errno));
				goto failed;
			}
		}
		else if (recv_addrinfo->ai_family == AF_INET6)
		{
			struct ipv6_mreq mreq;
			mreq.ipv6mr_multiaddr = ((struct sockaddr_in6 *) 
				udp->recv_addrinfo->ai_addr)->sin6_addr;
			mreq.ipv6mr_interface = ((struct sockaddr_in6 *) 
				recv_addrinfo->ai_addr)->sin6_scope_id;
			status = setsockopt (udp->socket, IPPROTO_IP, 
				IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq));
			if (status == -1)
			{
				cms_ns_error ("libcms", "setsockopt IPPROTO_IP IP_ADD_MEMBERSHIP error (%d,%s)", 
					errno, strerror (errno));
				goto failed;
			}
		}

		// navazeme socket na lokalni interface
		status = bind (udp->socket, udp->recv_addrinfo->ai_addr, udp->recv_addrinfo->ai_addrlen);
		if (status == -1)
		{
			cms_ns_error ("libcms", "bind error (%d,%s)", 
				errno, strerror (errno));
			goto failed;
		}

		freeaddrinfo (recv_addrinfo);
		recv_addrinfo = NULL;
	}

	// nastavime magicke cislo
	udp->magic = CMS_UDP_MAGIC;

	// pridame objekt do epollu
	_cms_udp_epoll_add (udp);

	// vratime objekt
	return udp;

failed:
	if (udp)
	{
		if (udp->socket != -1)
		{
			close (udp->socket);
		}
		if (udp->recv_port)
		{
			g_free (udp->recv_port);
		}
		if (udp->send_port)
		{
			g_free (udp->send_port);
		}
		if (udp->send_addrinfo)
		{
			freeaddrinfo (udp->send_addrinfo);
		}
		if (udp->recv_addrinfo)
		{
			freeaddrinfo (udp->recv_addrinfo);
		}
		if (udp->send_buffer)
		{
			cms_round_buffer_delete (udp->send_buffer);
		}
		if (udp->recv_buffer)
		{
			cms_round_buffer_delete (udp->recv_buffer);
		}
		g_free (udp);
	}
	
	if (send_addrinfo)
	{
		freeaddrinfo (send_addrinfo);
	}
	if (recv_addrinfo)
	{
		freeaddrinfo (recv_addrinfo);
	}
	
	return NULL;
}

void cms_udp_delete (cms_udp_t * udp)
{
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return;
	}

	// ukoncime komunikace
	cms_udp_shutdown (udp);

	// odstranime objekt z pollu
	_cms_udp_epoll_remove (udp);

	// smazeme magicke cislo
	udp->magic = 0;

	// uvolnime polozky objektu
	if (udp->socket != -1)
	{
		close (udp->socket);
	}
	if (udp->recv_port)
	{
		g_free (udp->recv_port);
	}
	if (udp->send_port)
	{
		g_free (udp->send_port);
	}
	if (udp->send_addrinfo)
	{
		freeaddrinfo (udp->send_addrinfo);
	}
	if (udp->recv_addrinfo)
	{
		freeaddrinfo (udp->recv_addrinfo);
	}
	while (udp->source_addrinfo)
	{
		freeaddrinfo ((struct addrinfo *)udp->source_addrinfo->data);
		udp->source_addrinfo = g_list_delete_link (udp->source_addrinfo, udp->source_addrinfo);
	}
	if (udp->send_buffer)
	{
		cms_round_buffer_delete (udp->send_buffer);
	}
	if (udp->recv_buffer)
	{
		cms_round_buffer_delete (udp->recv_buffer);
	}

	// uvolnime objekt		
	g_free (udp);
}

void cms_udp_init (cms_udp_t * udp)
{
	int status;
	
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return;
	}

	// zamkneme zamek epollu
	g_mutex_lock (udp_epoll_mutex);
	
	// poznamename provedeni inicializace
	udp->init = TRUE;
	
	// vytvorime epoll
	if (udp_epoll_fd == -1)
	{
		udp_epoll_fd = epoll_create (1);
		if (udp_epoll_fd == -1)
		{
			cms_ns_error ("libcms", "epoll_create error (%d,%s)", 
				errno, strerror (errno));
		}
		status = fcntl (udp_epoll_fd, F_SETFD, FD_CLOEXEC);
		if (status == -1)
		{
			cms_ns_error ("libcms", "fcntl F_SETFD FD_CLOEXEC error (%d,%s)", 
				errno, strerror (errno));
		}
		cms_ns_info ("libcms", "epoll_create (%d)", udp_epoll_fd);
	}

	// sputime epoll thread
	if (!udp_epoll_thread)
	{
		udp_epoll_thread = g_thread_create_full (
			cms_udp_epoll_thread, NULL, 0x100000,
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);
		if (!udp_epoll_thread)
		{
			cms_ns_error ("libcms", "g_thread_create_full error");
		}
	}

	// odemkneme zamek epollu
	g_mutex_unlock (udp_epoll_mutex);
}

void cms_udp_shutdown (cms_udp_t * udp)
{
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return;
	}

	// ukoncime cinnost socketu
	if (udp->socket != -1)
	{
		shutdown (udp->socket, SHUT_RDWR);
	}

	// ukoncime cinnost vysilaciho buffer
	if (udp->send_buffer)
	{
		cms_round_buffer_set_flags (udp->send_buffer, 0);
		cms_round_buffer_clear (udp->send_buffer);
	}

	// ukoncime cinnost prijimaciho buffer
	if (udp->recv_buffer)
	{
		cms_round_buffer_set_flags (udp->recv_buffer, 0);
		cms_round_buffer_clear (udp->recv_buffer);
	}
}

void cms_udp_set_time_to_live (cms_udp_t * udp, unsigned char ttl)
{
	int status;
	
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return;
	}

	status = setsockopt (udp->socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof (ttl));
	if (status == -1)
	{
		cms_ns_error ("libcms", "setsockopt IPPROTO_IP IP_MULTICAST_TTL error (%d,%s)", 
			errno, strerror (errno));
	}
}

void cms_udp_set_use_bonding (cms_udp_t * udp)
{
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return;
	}

	udp->bonding = 1;
}

void cms_udp_flush (cms_udp_t * udp)
{
	// zamkneme zamek
	g_mutex_lock (udp_epoll_mutex);

	// cekame na vyprazdneni vystupniho bufferu
	while (udp->pollout)
	{
		g_cond_wait (udp_epoll_cond, udp_epoll_mutex);
	}

	// odemkneme zamek
	g_mutex_unlock (udp_epoll_mutex);
}

int cms_udp_send (cms_udp_t * udp, const void * data, unsigned size)
{
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return -1;
	}

	// objekt neni inicializovany na vysilani
	if (!(udp->send_recv_flags & CMS_FLAG_SEND))
	{
		cms_ns_error ("libcms", "object is not initialized for sending");
		return -1;
	}
	
	// pokud je treba, inicializujeme objekt
	if (!udp->init)
	{
		cms_udp_init (udp);
	}
	
	// zapiseme data do bufferu
	size = cms_round_buffer_write (udp->send_buffer, data, size);

	// povolime vysilani dat
	_cms_udp_epoll_set (udp);

	return size;
}

int cms_udp_recv (cms_udp_t * udp, void * data, unsigned size)
{
	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return -1;
	}

	// objekt neni inicializovan na prijem
	if (!(udp->send_recv_flags & CMS_FLAG_RECV))
	{
		cms_ns_error ("libcms", "object is not initialized for receiving");
		return -1;
	}

	// objekt neni inicializovan na prime cteni
	if (udp->external_recv_buffer)
	{
		return -1;
	}

	// pokud je treba, inicializujeme objekt
	if (udp->init)
	{
		cms_udp_init (udp);
	}

	// precteme data z bufferu
	return cms_round_buffer_read (udp->recv_buffer, data, size);
}

int _cms_udp_send (cms_udp_t * udp, const void * data, unsigned size)
{
	unsigned char buffer[size+2];
	int size2;

	if (!udp || udp->magic != CMS_UDP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
		return -1;
	}

	if (size >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong size: %d", size);
		return -1;
	}

	if (!udp->send_addrinfo)
	{
		//cms_ns_error ("libcms", "unable to send data");
		return -1;
	}

	if (udp->bonding)
	{
		// zkopirujeme data a pridame na konec citac
		memcpy (buffer, data, size);
		buffer[size++] = udp->bonding_send_count & 0xff;
		buffer[size++] = (udp->bonding_send_count >> 8) & 0xff;
		data = buffer;
	}

	if (!udp->send_addrinfo)
	{
		//cms_error("unable to send data");
		return -1;
	}

	// odesleme data
	size2 = sendto (udp->socket, data, size, MSG_DONTWAIT | MSG_NOSIGNAL,
		udp->send_addrinfo->ai_addr, udp->send_addrinfo->ai_addrlen);

	if (size2 != (int) size)
	{
		cms_ns_error ("libcms", "sendto error (%d,%d,%d,%d,%s)", 
			udp->socket, size, size2, errno, strerror (errno));
		return -1;
	}

	if (udp->bonding)
	{
		udp->bonding_send_count++;
	}		
	
	return size;
}

gboolean _cms_udp_test_source_addr (cms_udp_t * udp, struct sockaddr * addr)
{
	struct addrinfo * addrinfo;
	GList * link;
	
	if (!udp->source_addrinfo)
	{
		return TRUE;
	}

	link = udp->source_addrinfo;
	while (link)
	{
		addrinfo = (struct addrinfo *) link->data;

		if (addr->sa_family == addrinfo->ai_addr->sa_family)
		{
			if (addr->sa_family == AF_INET)
			{
				if (((struct sockaddr_in *) addr)->sin_addr.s_addr
					== ((struct sockaddr_in *) addrinfo->ai_addr)->sin_addr.s_addr)
				{
					return TRUE;
				}
			}
			else if (addr->sa_family == AF_INET6)
			{
				if (IN6_ARE_ADDR_EQUAL(&((struct sockaddr_in6 *) addr)->sin6_addr,
					&((struct sockaddr_in6 *) addrinfo->ai_addr)->sin6_addr))
				{
					return TRUE;
				}
			}
		}

		link = link->next;
	}

	return FALSE;
}

int _cms_udp_recv (cms_udp_t * udp, void * data, unsigned maxsize)
{
	char tmp[64];
	struct sockaddr * addr = (struct sockaddr *) tmp;
	socklen_t addrlen = sizeof (tmp);
	GTimeVal time;
	int size = -1;

	if (maxsize == 0 || maxsize >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong maxsize: %d", maxsize);
		return -1;
	}
	
	if (!udp->recv_addrinfo)
	{
		size = recvfrom (udp->socket, data, maxsize, 
			MSG_DONTWAIT | MSG_NOSIGNAL, addr, &addrlen);
		cms_ns_error ("libcms", "unable to receive data");
		return -1;
	}

	// pouzivame citac
	if (udp->bonding)
	{
		unsigned char buffer[maxsize + 2];
		unsigned short count;

		// prijmeme data
		size = recvfrom (udp->socket, (void *) buffer, maxsize + 2, 
			MSG_DONTWAIT | MSG_NOSIGNAL, addr, &addrlen);
		if (size == -1)
		{
			cms_ns_error ("libcms", "recvfrom error (%d,%d,%d,%d,%s)", 
				udp->socket, maxsize, size, errno, strerror (errno));
			return -1;
		}

		// otestujeme zdrojovou adresu
		if (!_cms_udp_test_source_addr (udp, addr))
		{
			return -1;
		}

		// kratka zprava
		if (size < 2)
		{
			return -1;
		}

		// precteme citac na konci packetu
		count = buffer[--size];
		count = (count << 8) | buffer[--size];
		g_get_current_time (&time);

		// uz deset vterin nebylo nic prijato
		if (udp->bonding_recv_time + 10 < time.tv_sec)
		{
			udp->bonding_recv_count = count;
			udp->bonding_recv_time = time.tv_sec;
		}

		// prisel novy packet
		else if (count > udp->bonding_recv_count 
			|| (count < 100 && udp->bonding_recv_count > 65436))
		{
			udp->bonding_recv_count = count;
			udp->bonding_recv_time = time.tv_sec;
		}

		// prisel stary packet, zahodime ho
		else
		{
			return -1;
		}

		memcpy (data, buffer, size);
	}

	// normalni prijem
	else
	{
		size = recvfrom (udp->socket, data, maxsize, 
			MSG_DONTWAIT | MSG_NOSIGNAL, addr, &addrlen);
		if (size == -1)
		{
			cms_ns_error ("libcms", "recvfrom error (%d,%d,%d,%d,%s)", 
				udp->socket, maxsize, size, errno, strerror (errno));
			return -1;
		}

		// otestujeme zdrojovou adresu
		if (!_cms_udp_test_source_addr (udp, addr))
		{
			return -1;
		}
	}

	// dame na konec nulu kvuli prenosum textu
	if (size < 0 && maxsize > 0)
	{
		((char *)data)[0] = 0;
	}
	else if (size >= 0 && size < maxsize)
	{
		((char *)data)[size] = 0;
	}
	
	return size;
}

void _cms_udp_epoll_add (cms_udp_t * udp)
{
	struct epoll_event event;
	int status;

	// zamkneme zamek
	g_mutex_lock (udp_epoll_mutex);

	// vytvorime epoll
	if (udp_epoll_fd == -1)
	{
		udp_epoll_fd = epoll_create (1);
		if (udp_epoll_fd == -1)
		{
			cms_ns_error ("libcms", "epoll_create error (%d,%s)", 
				errno, strerror (errno));
		}
		status = fcntl (udp_epoll_fd, F_SETFD, FD_CLOEXEC);
		if (status == -1)
		{
			cms_ns_error ("libcms", "fcntl F_SETFD FD_CLOEXEC error (%d,%s)", 
				errno, strerror (errno));
		}
		cms_ns_info ("libcms", "epoll_create (%d)", udp_epoll_fd);
	}

	// pridame udp objekt do seznamu
	udp_epoll_list = g_list_append (udp_epoll_list, udp);

	// povolime signal k prijmu
	udp->pollout = FALSE;
	memset (&event, 0, sizeof (event));	
	event.events = EPOLLIN;
	event.data.ptr = udp;
	status = epoll_ctl (udp_epoll_fd, EPOLL_CTL_ADD, udp->socket, &event);
	if (status == -1)
	{
		cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_ADD error (%d,%d,%d,%s)", 
			udp->socket, udp_epoll_fd, errno, strerror (errno));
	}

	// odemkneme zamek
	g_mutex_unlock (udp_epoll_mutex);
}

void _cms_udp_epoll_remove (cms_udp_t * udp)
{
	// zamkneme zamek
	g_mutex_lock (udp_epoll_mutex);

	// nevyhodime objekt z pollu, ale pockame, az to za nas udela poll thread, 
	// pokud je jiz ukoncene spojeni, melo by k tomu dojit bezprostredne
	while (g_list_find (udp_epoll_list, udp))
	{
		g_cond_wait (udp_epoll_cond, udp_epoll_mutex);
	}

	// odemkneme zamek
	g_mutex_unlock (udp_epoll_mutex);
}

void _cms_udp_epoll_set (cms_udp_t * udp)
{
	struct epoll_event event;
	int status;

	// zamkneme zamek
	g_mutex_lock (udp_epoll_mutex);

	// jsou data k odeslani
	if (!cms_round_buffer_is_empty (udp->send_buffer))
	{
		if (!udp->pollout)
		{
			// povolime vysilani
			udp->pollout = TRUE;
			memset (&event, 0, sizeof (event));
			event.events = EPOLLIN | EPOLLOUT;
			event.data.ptr = udp;
			status = epoll_ctl (udp_epoll_fd, EPOLL_CTL_MOD, udp->socket, &event);
			if (status == -1)
			{
				cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_MOD error (%d,%d,%d,%s)", 
					udp->socket, udp_epoll_fd, errno, strerror (errno));
			}
		}
	}

	// nejsou data k odeslani
	else
	{
		if (udp->pollout)
		{
			// zakazeme vysilani
			udp->pollout = FALSE;
			memset (&event, 0, sizeof (event));
			event.events = EPOLLIN;
			event.data.ptr = udp;
			status = epoll_ctl (udp_epoll_fd, EPOLL_CTL_MOD, udp->socket, &event);
			if (status == -1)
			{
				cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_MOD error (%d,%d,%d,%s)", 
					udp->socket, udp_epoll_fd, errno, strerror (errno));
			}
		}
	}
		
	// odemkneme zamek
	g_mutex_unlock (udp_epoll_mutex);
	g_cond_broadcast (udp_epoll_cond);
}

void _cms_udp_epoll_in (cms_udp_t * udp)
{
	char buffer[MSGMAX];
	int size, status;

	// prijmeme data
	size = _cms_udp_recv (udp, buffer, sizeof (buffer));
	if (size >= 0)
	{
		if (udp->external_recv_buffer)
		{
			status = cms_round_buffer_write (udp->external_recv_buffer, buffer, size);
			if (status != size)
			{
				cms_ns_error ("libcms", "cms_round_buffer_write error");
			}
		}
		else if (udp->recv_buffer)
		{
			status = cms_round_buffer_write (udp->recv_buffer, buffer, size);
			if (status != size)
			{
				cms_ns_error ("libcms", "cms_round_buffer_write error");
			}
		}
	}
}

void _cms_udp_epoll_out (cms_udp_t * udp)
{
	char buffer[MSGMAX];
	int size;

	// prectu data z bufferu
	size = cms_round_buffer_read (udp->send_buffer, buffer, sizeof (buffer));
	if (size >= 0)
	{
		// odvysilame data
		_cms_udp_send (udp, buffer, size);
	}

	// pokud nejsou data, zakazeme dalsi vysilani
	_cms_udp_epoll_set (udp);
}

void _cms_udp_epoll_hup (cms_udp_t * udp)
{
	int status;

	// ukoncime spojeni
	cms_udp_shutdown (udp);

	// zamkneme zamek
	g_mutex_lock (udp_epoll_mutex);

	// vyhodime objekt ze seznamu
	udp_epoll_list = g_list_remove (udp_epoll_list, udp);

	// vyhodime socket z pollu
	status = epoll_ctl (udp_epoll_fd, EPOLL_CTL_DEL, udp->socket, NULL);
	if (status == -1)
	{
		cms_ns_error ("libcms", "epoll_ctl EPOLL_CTL_DEL error (%d,%d,%d,%s)", 
			udp->socket, udp_epoll_fd, errno, strerror (errno));
	}
	
	// odemkneme zamek
	g_mutex_unlock (udp_epoll_mutex);
	g_cond_broadcast (udp_epoll_cond);
}

gpointer cms_udp_epoll_thread (gpointer data)
{
	struct epoll_event event;
	cms_udp_t * udp = NULL;
	int status;

	while (TRUE)
	{
		status = epoll_wait (udp_epoll_fd, &event, 1, -1);
			
		if (status == 1)
		{
			//cms_ns_debug ("libcms", "epoll event");
			udp = (cms_udp_t *) event.data.ptr;
			if (!udp || udp->magic != CMS_UDP_MAGIC)
			{
				cms_ns_error ("libcms", "wrong magic %08X", udp->magic);
			}
			else
			{
				if (event.events & EPOLLIN)
				{
					//cms_ns_debug ("libcms", "EPOLLIN");
					_cms_udp_epoll_in (udp);
				}
				if (event.events & EPOLLOUT)
				{
					//cms_ns_debug ("libcms", "EPOLLOUT");
					_cms_udp_epoll_out (udp);
				}
				if (event.events & EPOLLERR)
				{
					//cms_ns_debug ("libcms", "EPOLLERR");
				}
				if (event.events & EPOLLHUP)
				{
					//cms_ns_debug ("libcms", "EPOLLHUP");
					_cms_udp_epoll_hup (udp);
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
				udp_epoll_fd, errno, strerror (errno));
		}
	}

	return NULL;
}

