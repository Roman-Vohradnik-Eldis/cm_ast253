#include "cms_ip.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_IP_MAGIC 0x72060906

void _cms_ip_time_add (cms_ip_t * ip);
void _cms_ip_time_remove (cms_ip_t * ip);
gpointer cms_ip_time_thread (gpointer data);

static GMutex * ip_time_mutex = NULL;
static GThread * ip_time_thread = NULL;
static GList * ip_time_list = NULL;

cms_ip_t * cms_ip_new (int send_recv_flags, const char * descriptors)
{
	cms_ip_t * ip;
	char descriptor[1024];
	char text[1024];
	const char * descrptr;
	const char * str;
	char * ptr;

	// alokujeme zamek
	if (!ip_time_mutex)
	{
		ip_time_mutex = g_mutex_new ();
	}
	
	// alokujeme objekt
	ip = (cms_ip_t *) g_malloc0 (sizeof (cms_ip_t));
	if (!ip)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	// inicializujeme promenne
	ip->run = TRUE;
	ip->init = FALSE;
	ip->send_recv_flags = send_recv_flags;
	ip->descriptors = g_strdup (descriptors);
	ip->max_buffer_size = 0;
	ip->max_buffer_time = 0;

	// vytvorime mutext na navazovani spojeni	
	ip->mutex = g_mutex_new ();
	if (!ip->mutex)
	{
		cms_ns_error ("libcms", "g_mutex_new error");
		goto failed;
	}

	// prohledame descriptor
	descrptr = descriptors;
	while (sscanf (descrptr, "%s", descriptor) == 1)
	{
		descrptr = strstr (descrptr, descriptor) + strlen (descriptor);

		// rozsekame retezec na pomlcky 
		char ** item = g_strsplit (descriptor, ";", 0);
		if (!item || !item[0])
		{
			cms_ns_error ("libcms", "g_strsplit error");
			goto failed;
		}

		// rozsekame retezec na stredniky
		if (!item[1])
		{
			g_strfreev (item);
			item = g_strsplit (descriptor, ":", 0);
			if (!item || !item[0])
			{
				cms_ns_error ("libcms", "g_strsplit error");
				goto failed;
			}
		}

		// buffer:0x1000000:10
		if (!strcmp (item[0], "buffer"))
		{
			if (!item[1] || !item[2])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s)", 
					descriptors, descriptor);
				goto failed;
			}

			ip->max_buffer_size = strtol (item[1], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			ip->max_buffer_time = strtol (item[2], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}
		}

		// uvolnime pamet
		g_strfreev (item);
	}
	
	// neni znama maximalni velikost bufferu,
	// pokusime se precist z promennych prostredi
	if (!ip->max_buffer_size)
	{
		str = g_getenv ("MAX_BUFFER_SIZE");
		if (str)
		{
			ip->max_buffer_size = strtol (str, &ptr, 0);
			if (*ptr)
			{
				ip->max_buffer_size = 0;
			}
		}

		str = g_getenv ("MAX_BUFFER_TIME");
		if (str)
		{
			ip->max_buffer_time = strtol (str, &ptr, 0);
			if (*ptr)
			{
				ip->max_buffer_time = 0;
			}
		}
	}

	// neni znama maximalni velikost bufferu,
	// dosadime defaultni hodnotu
	if (!ip->max_buffer_size)
	{
		ip->max_buffer_size = CMS_IP_MAX_BUFFER_SIZE;
	}

	// alokujeme buffer na prijem
	g_snprintf (text, sizeof (text), "ip receive buffer %s", descriptors);
	ip->recv_buffer = cms_round_buffer_new_with_time_block (
		text, ip->max_buffer_size, ip->max_buffer_time, 
		CMS_ROUND_BUFFER_READ_BLOCK | CMS_ROUND_BUFFER_WRITE_GROW 
		| CMS_ROUND_BUFFER_WRITE_THROW);
	if (!ip->recv_buffer)
	{
		goto failed;
	}

	// prohledame descriptor
	descrptr = descriptors;
	while (sscanf (descrptr, "%s", descriptor) == 1)
	{
		descrptr = strstr (descrptr, descriptor) + strlen (descriptor);

		// rozsekame retezec na pomlcky 
		char ** item = g_strsplit (descriptor, ";", 0);
		if (!item || !item[0])
		{
			cms_ns_error ("libcms", "g_strsplit error");
			goto failed;
		}

		// rozsekame retezec na stredniky
		if (!item[1])
		{
			g_strfreev (item);
			item = g_strsplit (descriptor, ":", 0);
			if (!item || !item[0])
			{
				cms_ns_error ("libcms", "g_strsplit error");
				goto failed;
			}
		}

		// client:192.168.100.200:0x2010:10
		// client-text:192.168.100.200:0x2010:10
		// client-raw:192.168.100.200:0x2010:10
		if (!strcmp (item[0], "client")
			|| !strcmp (item[0], "client-text")
			|| !strcmp (item[0], "client-raw"))
		{
			cms_client_t * client;
			
			if (!item[1] || !item[2] || !item[3] || item[4])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s)", 
					descriptors, descriptor);
				goto failed;
			}

			char * addr = item[1];
			char * port = item[2];

			int ping = strtol (item[3], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			if (!strcmp (item[0], "client-text"))
			{
				client = cms_client_new (send_recv_flags, addr, port, ping, 
					CMS_TCP_MODE_TEXT, CMS_CLIENT_MAX_BUFFER_SIZE, ip->recv_buffer);
			}
			else if (!strcmp (item[0], "client-raw"))
			{
				client = cms_client_new (send_recv_flags, addr, port, ping, 
					CMS_TCP_MODE_RAW, CMS_CLIENT_MAX_BUFFER_SIZE, ip->recv_buffer);
			}
			else
			{
				client = cms_client_new (send_recv_flags, addr, port, ping, 
					CMS_TCP_MODE_ELDIS, CMS_CLIENT_MAX_BUFFER_SIZE, ip->recv_buffer);
			}
			
			if (!client)
			{
				goto failed;
			}
	
			if (!ip->client)
			{
				ip->client = g_ptr_array_new ();
			}
			
			g_ptr_array_add (ip->client, client);
		}

		// server:0.0.0.0:0x2001:10
		// server-text:0.0.0.0:0x2001:10
		// server-raw:0.0.0.0:0x2001:10
		else if (!strcmp (item[0], "server")
			|| !strcmp (item[0], "server-text")
			|| !strcmp (item[0], "server-raw"))
		{
			cms_server_2_t * server;
			
			if (!item[1] || !item[2] || !item[3] || item[4])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s)", 
					descriptors, descriptor);
				goto failed;
			}

			char * addr = item[1];
			char * port = item[2];

			int ping = strtol (item[3], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			if (!strcmp (item[0], "server-text"))
			{
				server = cms_server_2_new (send_recv_flags, addr, port, ping, 
					CMS_TCP_MODE_TEXT, CMS_CLIENT_MAX_BUFFER_SIZE,
					NULL, NULL, NULL, NULL, ip->recv_buffer);
			}
			else if (!strcmp (item[0], "server-raw"))
			{
				server = cms_server_2_new (send_recv_flags, addr, port, ping, 
					CMS_TCP_MODE_RAW, CMS_CLIENT_MAX_BUFFER_SIZE,
					NULL, NULL, NULL, NULL, ip->recv_buffer);
			}
			else
			{
				server = cms_server_2_new (send_recv_flags, addr, port, ping, 
					CMS_TCP_MODE_ELDIS, CMS_CLIENT_MAX_BUFFER_SIZE,
					NULL, NULL, NULL, NULL, ip->recv_buffer);
			}

			if (!server)
			{
				goto failed;
			}

			if (!ip->server)
			{
				ip->server = g_ptr_array_new ();
			}
			
			g_ptr_array_add (ip->server, server);
		}

		// udp:0.0.0.0:0x2020:192.168.100.200:0x2020
		// udp-bonding:0.0.0.0:0x2020:192.168.100.200:0x2020
		else if (!strcmp (item[0], "udp")
			|| !strcmp (item[0], "udp-bonding"))
		{
			cms_udp_t * udp;
			
			if (!item[1] || !item[2] || !item[3] || !item[4])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			char * recv_local_addr = item[1];
			char * recv_local_port = item[2];
			char * send_remote_addr = item[3];
			char * send_remote_port = item[4];

			udp = cms_udp_new (send_recv_flags, 
				recv_local_addr, recv_local_port, 
				send_remote_addr, send_remote_port, 
				(const char **) (item + 5), ip->recv_buffer);

			if (!udp)
			{
				goto failed;
			}
			
			if (!strcmp (item[0], "udp-bonding"))
			{
				cms_udp_set_use_bonding (udp);
			}

			if (!ip->udp)
			{
				ip->udp = g_ptr_array_new ();
			}
			
			g_ptr_array_add (ip->udp, udp);
		}

		// mcudp:239.255.0.52:192.168.5.1:0x2020:239.255.52:192.168.5.1:0x2020
		// mcudp-bonding:239.255.0.52:192.168.5.1:0x2020:239.255.52:192.168.5.1:0x2020
		else if (!strcmp (item[0], "mcudp")
			|| !strcmp (item[0], "mcudp-bonding"))
		{
			cms_udp_t * udp;
			
			if (!item[1] || !item[2] || !item[3] || !item[4] 
				|| !item[5] || !item[6] || item[7])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			char * recv_multicast_addr = item[1];
			char * recv_local_addr = item[2];
			char * recv_port = item[3];
			char * send_multicast_addr = item[4];
			char * send_local_addr = item[5];
			char * send_port = item[6];

			udp = cms_udp_new_multicast (send_recv_flags, 
				recv_multicast_addr, recv_local_addr, recv_port, 
				send_multicast_addr, send_local_addr, send_port, ip->recv_buffer);
			if (!udp)
			{
				goto failed;
			}
			
			if (!strcmp (item[0], "mcudp-bonding"))
			{
				cms_udp_set_use_bonding (udp);
			}

			if (!ip->udp)
			{
				ip->udp = g_ptr_array_new ();
			}
			
			g_ptr_array_add (ip->udp, udp);
		}

		// file-read:path:maxsize:waittime
		else if (!strcmp (item[0], "file-read"))
		{
			cms_file_t * file;
			unsigned maxsize = 0;
			unsigned timeout = 0;

			if (!item[1])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			if (item[2])
			{
				maxsize = strtol (item[2], &ptr, 0);
				if (*ptr)
				{
					cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
						descriptors, descriptor, ptr);
					goto failed;
				}

				if (item[3])
				{
					timeout = strtol (item[3], &ptr, 0);
					if (*ptr)
					{
						cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
							descriptors, descriptor, ptr);
						goto failed;
					}

					if (item[4])
					{
						cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
						goto failed;
					}
				}
			}

			file = cms_file_new_read (item[1], maxsize, timeout);
			if (!file)
			{
				goto failed;
			}

			if (ip->file)
			{
				cms_ns_error ("libcms", "file must be exclusive");
				goto failed;
			}
			
			ip->file = file;
		}

		// file-write:path
		else if (!strcmp (item[0], "file-write"))
		{
			cms_file_t * file;

			if (!item[1] || item[2])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			file = cms_file_new_write (item[1]);
			if (!file)
			{
				goto failed;
			}
			
			if (ip->file)
			{
				cms_ns_error ("libcms", "file must be exclusive");
				goto failed;
			}
			
			ip->file = file;
		}

		// text-read:path:maxsize:waittime
		else if (!strcmp (item[0], "text-read"))
		{
			cms_text_t * text;
			unsigned maxsize = 0;
			unsigned timeout = 0;

			if (!item[1])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			if (item[2])
			{
				maxsize = strtol (item[2], &ptr, 0);
				if (*ptr)
				{
					cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
						descriptors, descriptor, ptr);
					goto failed;
				}

				if (item[3])
				{
					timeout = strtol (item[3], &ptr, 0);
					if (*ptr)
					{
						cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
							descriptors, descriptor, ptr);
						goto failed;
					}

					if (item[4])
					{
						cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
						goto failed;
					}
				}
			}

			text = cms_text_new_read (item[1], maxsize, timeout);
			if (!text) goto failed;

			if (ip->text)
			{
				cms_ns_error ("libcms", "text must be exclusive");
				goto failed;
			}
			
			ip->text = text;
		}

		// text-write:path
		else if (!strcmp (item[0], "text-write"))
		{
			cms_text_t * text;

			if (!item[1] || item[2])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			text = cms_text_new_write (item[1]);
			if (!text)
			{
				goto failed;
			}
			
			if (ip->text)
			{
				cms_ns_error ("libcms", "text must be exclusive");
				goto failed;
			}
			
			ip->text = text;
		}

#ifdef G_OS_UNIX
		// com:device:speed:data_bits:stop_bits:parity:break_time:msg_start:msg_stop
		// com-komar:device:speed:data_bits:stop_bits:parity:break_time
		// com-vitek:device:speed:data_bits:stop_bits:parity:break_time
		// com-ams:device:speed:data_bits:stop_bits:parity:break_time
		else if (!strcmp (item[0], "com")
			|| !strcmp (item[0], "com-komar")
			|| !strcmp (item[0], "com-vitek")
			|| !strcmp (item[0], "com-ams"))
		{
			cms_com_t * com;
			
			if (!item[1] || !item[2] || !item[3] || !item[4] || !item[5] || !item[6])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			char * device = item[1];
			
			int speed = strtol (item[2], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			int data_bits = strtol (item[3], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			int stop_bits = strtol (item[4], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			int parity = strtol (item[5], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			int break_time = strtol (item[6], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}

			if (!strcmp (item[0], "com-komar"))
			{
				if (item[7])
				{
					cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
					goto failed;
				}

				com = cms_com_new_komar (device, speed, 
					data_bits, stop_bits, parity, break_time);
			}

			else if (!strcmp (item[0], "com-vitek"))
			{
				if (item[7])
				{
					cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
					goto failed;
				}

				com = cms_com_new_vitek (device, speed, 
					data_bits, stop_bits, parity, break_time);
			}

			else if (!strcmp (item[0], "com-ams"))
			{
				if (item[7])
				{
					cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
					goto failed;
				}

				com = cms_com_new_ams (device, speed, 
					data_bits, stop_bits, parity, break_time);
			}
			
			else
			{
				if (!item[7] || !item[8] || item[9])
				{
					cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
					goto failed;
				}
				
				ptr = item[7];
				char * msg_start = g_malloc0 (strlen (ptr) + 1);
				char * start = msg_start;
				while (*ptr)
				{
					if (!g_ascii_isxdigit (ptr[0]) && !g_ascii_isxdigit (ptr[1]))
					{
						cms_ns_error ("libcms", "wrong characters (%c%c) in descriptor (%s)",
							ptr[0], ptr[1], descriptor);
						goto failed;
					}
					char c = g_ascii_xdigit_value (ptr[0]) * 16 
						+ g_ascii_xdigit_value (ptr[1]);
					*start++ = c;
					ptr += 2;
				}

				ptr = item[8];
				char * msg_stop = g_malloc0 (strlen (ptr) + 1);
				char * stop = msg_stop;
				while (*ptr)
				{
					if (!g_ascii_isxdigit (ptr[0]) && !g_ascii_isxdigit (ptr[1]))
					{
						cms_ns_error ("libcms", "wrong characters (%c%c) in descriptor (%s)",
							ptr[0], ptr[1], descriptor);
						goto failed;
					}
					char c = g_ascii_xdigit_value (ptr[0]) * 16
						+ g_ascii_xdigit_value (ptr[1]);
					*stop++ = c;
					ptr += 2;
				}

				com = cms_com_new_start_stop (device, speed, data_bits, stop_bits, 
					parity, break_time, msg_start, msg_stop);
				g_free (msg_start);
				g_free (msg_stop);
			}

			if (!com)
			{
				goto failed;
			}

			if (ip->com)
			{
				cms_ns_error ("libcms", "com must be exclusive");
				goto failed;
			}
			
			ip->com = com;
		}
#endif
		// buffer:0x1000000:10
		else if (!strcmp (item[0], "buffer"))
		{
		}

		// status:PSR-VIDEO:10
		else if (!strcmp (item[0], "status"))
		{
			if (!item[1] || !item[2] || item[3])
			{
				cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
				goto failed;
			}

			if (item[1][0])
			{
				ip->status_name = g_strdup (item[1]);
			}
			else
			{
				ip->status_name = g_strdup ("noname");
			}
			

			ip->status_time = strtol (item[2], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "wrong descriptor (%s,%s,%s)", 
					descriptors, descriptor, ptr);
				goto failed;
			}
		}

		else
		{
			cms_ns_error ("libcms", "wrong descriptor (%s)", descriptors);
			goto failed;
		}

		// uvolnime pamet
		g_strfreev (item);
	}

	if (ip->file && (ip->client || ip->server || ip->udp || ip->text || ip->com))
	{
		cms_ns_error ("libcms", "file must be exclusive");
		goto failed;
	}
	if (ip->text && (ip->client || ip->server || ip->udp || ip->file || ip->com))
	{
		cms_ns_error ("libcms", "text must be exclusive");
		goto failed;
	}
	if (ip->com && (ip->client || ip->server || ip->udp || ip->file || ip->text))
	{
		cms_ns_error ("libcms", "com must be exclusive");
		goto failed;
	}

	if (ip->status_time)
	{
		ip->send_bytes = g_malloc0 (ip->status_time * sizeof (unsigned));
		if (!ip->send_bytes)
		{
			cms_ns_error ("libcms", "g_malloc0 error");
			goto failed;
		}
		ip->send_packets = g_malloc0 (ip->status_time * sizeof (unsigned));
		if (!ip->send_packets)
		{
			cms_ns_error ("libcms", "g_malloc0 error");
			goto failed;
		}
		ip->recv_bytes = g_malloc0 (ip->status_time * sizeof (unsigned));
		if (!ip->recv_bytes)
		{
			cms_ns_error ("libcms", "g_malloc0 error");
			goto failed;
		}
		ip->recv_packets = g_malloc0 (ip->status_time * sizeof (unsigned));
		if (!ip->recv_packets)
		{
			cms_ns_error ("libcms", "g_malloc0 error");
			goto failed;
		}

		if (ip->client)
		{
			ip->status_port_1 = ((cms_client_t *) ip->client->pdata[0])->port;
			ip->status_port_2 = ((cms_client_t *) ip->client->pdata[0])->port;
		}
		else if (ip->server)
		{
			ip->status_port_1 = ((cms_server_2_t *) ip->server->pdata[0])->port;
			ip->status_port_2 = ((cms_server_2_t *) ip->server->pdata[0])->port;
		}
		else if (ip->udp)
		{
			ip->status_port_1 = ((cms_udp_t *) ip->udp->pdata[0])->recv_port;
			ip->status_port_2 = ((cms_udp_t *) ip->udp->pdata[0])->send_port;			
		}
	}

	ip->magic = CMS_IP_MAGIC;
	_cms_ip_time_add (ip);
	return ip;

failed:
	if (ip)
	{
		cms_ip_delete (ip);
	}
	
	return NULL;
}

void cms_ip_delete (cms_ip_t * ip)
{
	int i;

	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return;
	}

	cms_ip_shutdown (ip);

	ip->magic = 0;

	if (ip->client)
	{
		for (i = 0; i < ip->client->len; i++)
		{
			cms_client_delete ((cms_client_t *) ip->client->pdata[i]);
		}
		g_ptr_array_free (ip->client, TRUE);
	}

	if (ip->server)
	{
		for (i = 0; i < ip->server->len; i++)
		{
			cms_server_2_delete ((cms_server_2_t *) ip->server->pdata[i]);
		}
		g_ptr_array_free (ip->server, TRUE);
	}

	if (ip->udp)
	{
		for (i = 0; i < ip->udp->len; i++)
		{
			cms_udp_delete ((cms_udp_t *) ip->udp->pdata[i]);
		}
		g_ptr_array_free (ip->udp, TRUE);
	}

	if (ip->file)
	{
		cms_file_delete (ip->file);
	}

	if (ip->text)
	{
		cms_text_delete (ip->text);
	}

#ifdef G_OS_UNIX
	if (ip->com)
	{
		cms_com_delete (ip->com);
	}
#endif

	if (ip->recv_buffer)
	{
		cms_round_buffer_delete (ip->recv_buffer);
	}

	if (ip->status_name)
	{
		g_free (ip->status_name);
	}
	
	if (ip->send_bytes)
	{
		g_free (ip->send_bytes);
	}

	if (ip->send_packets)
	{
		g_free (ip->send_packets);
	}

	if (ip->recv_bytes)
	{
		g_free (ip->recv_bytes);
	}

	if (ip->recv_packets)
	{
		g_free (ip->recv_packets);
	}

	if (ip->mutex)
	{
		g_mutex_free (ip->mutex);
	}
	
	if (ip->descriptors)
	{
		g_free (ip->descriptors);
	}
	
	g_free (ip);
}

void cms_ip_shutdown (cms_ip_t * ip)
{
	int i;

	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return;
	}

	if (!ip->run)
	{
		cms_ns_warning ("libcms", "seems to be shutted down already");
		return;
	}
	
	ip->run = FALSE;

	_cms_ip_time_remove (ip);
	
	cms_round_buffer_set_flags (ip->recv_buffer, 0);

	if (ip->client)
	{
		for (i = 0; i < ip->client->len; i++)
		{
			cms_client_shutdown ((cms_client_t *) ip->client->pdata[i]);
		}
	}
	
	if (ip->client_thread)
	{
		for (i = 0; i < ip->client_thread->len; i++)
		{
			g_thread_join ((GThread *) ip->client_thread->pdata[i]);
		}
	}

	if (ip->server)
	{
		for (i = 0; i < ip->server->len; i++)
		{
			cms_server_2_shutdown ((cms_server_2_t *) ip->server->pdata[i]);
		}
	}

	if (ip->udp)
	{
		for (i = 0; i < ip->udp->len; i++)
		{
			cms_udp_shutdown ((cms_udp_t *) ip->udp->pdata[i]);
		}
	}

	if (ip->file)
	{
		cms_file_close (ip->file);
	}

	if (ip->text)
	{
		cms_text_close (ip->text);
	}

#ifdef G_OS_UNIX
	if (ip->com)
	{
		cms_com_close (ip->com);
	}
#endif
}

gboolean cms_ip_is_connected (cms_ip_t * ip)
{
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return 0;
	}

	int i;

	if (ip->client)
	{
		for (i = 0; i < ip->client->len; i++)
		{
			if (cms_client_is_connected ((cms_client_t *) ip->client->pdata[i]))
			{
				return TRUE;
			}
		}
	}

	if (ip->server)
	{
		for (i = 0; i < ip->server->len; i++)
		{
			if (cms_server_2_is_connected ((cms_server_2_t *) ip->server->pdata[i]))
			{
				return TRUE;
			}
		}
	}

	if (ip->udp)
	{
		return TRUE;
	}

	if (ip->file)
	{
		return TRUE;
	}

	if (ip->text)
	{
		return TRUE;
	}

#ifdef G_OS_UNIX
	if (ip->com)
	{
		return TRUE;
	}
#endif

	return FALSE;
}

gchar * cms_ip_get_remote_addr (cms_ip_t * ip)
{
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return 0;
	}

	GString * string = g_string_new ("");
	int i;

	if (ip->client)
	{
		for (i = 0; i < ip->client->len; i++)
		{
			const char * addr = cms_client_get_remote_addr ((cms_client_t *) ip->client->pdata[i]);
			if (string->len)
			{
				g_string_append (string, " ");
			}
			g_string_append (string, addr);
		}
	}

	if (ip->server)
	{
		for (i = 0; i < ip->server->len; i++)
		{
			char * addr = cms_server_2_get_remote_addr ((cms_server_2_t *) ip->server->pdata[i], 0);
			if (addr)
			{
				if (string->len)
				{
					g_string_append (string, " ");
				}
				g_string_append (string, addr);
				g_free (addr);
			}
		}
	}

	return g_string_free (string, FALSE);
}

unsigned cms_ip_get_recv_buffer_fill_space (cms_ip_t * ip)
{
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return 0;
	}

	return cms_round_buffer_get_fill_space (ip->recv_buffer);
}

unsigned cms_ip_get_recv_buffer_messages (cms_ip_t * ip)
{
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return 0;
	}

	return cms_round_buffer_get_messages (ip->recv_buffer);
}

int cms_ip_send (cms_ip_t * ip, const void * data, unsigned size)
{
	int i;
	
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return 0;
	}

	if (size >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong size: %d", size);
		return -1;
	}

	// objekt neni inicializovany na vysilani
	if (!(ip->send_recv_flags & CMS_FLAG_SEND))
	{
		cms_ns_error ("libcms", "object is not initialized for sending %s", 
			ip->descriptors);
		return -1;
	}
	
	// pokud je treba, inicializujeme objekt
	cms_ip_init (ip);
	
	// cinnost objektu jiz bylo ukoncena
	if (!ip->run)
	{
		return -1;
	}
	
	// pricteme data
	if (ip->status_time)
	{
		ip->send_bytes[0] += size;
		ip->send_packets[0] += 1;
	}

	if (ip->file)
	{
		return cms_file_write (ip->file, data, size);
	}

	if (ip->text)
	{
		return cms_text_write (ip->text, data, size);
	}

#ifdef G_OS_UNIX
	if (ip->com)
	{
		return cms_com_write (ip->com, data, size);
	}
#endif

	if (ip->client)
	{
		for (i = 0; i < ip->client->len; i++)
		{
			cms_client_send ((cms_client_t *) ip->client->pdata[i], data, size);
		}
	}

	if (ip->server)
	{
		for (i = 0; i < ip->server->len; i++)
		{
			cms_server_2_send ((cms_server_2_t *) ip->server->pdata[i], data, size, 0);
		}
	}

	if (ip->udp)
	{
		for (i = 0; i < ip->udp->len; i++)
		{
			cms_udp_send ((cms_udp_t *) ip->udp->pdata[i], data, size);
		}
	}
	
	return size;
}

int cms_ip_recv (cms_ip_t * ip, void * data, unsigned maxsize)
{
	int size;
	
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return -1;
	}

	if (maxsize == 0 || maxsize >= 0x10000)
	{
		cms_ns_error ("libcms", "wrong maxsize: %d", maxsize);
		return -1;
	}

	// objekt neni inicializovan na prijem
	if (!(ip->send_recv_flags & CMS_FLAG_RECV))
	{
		cms_ns_error ("libcms", "object is not initialized for receiving (%s)", 
			ip->descriptors);
		return -1;
	}

	// pokud je treba, inicializujeme objekt
	cms_ip_init (ip);
	
	// cinnost objektu jiz bylo ukoncena
	if (!ip->run)
	{
		return -1;
	}

	// pocitame data
	if (ip->file)
	{
		size = cms_file_read (ip->file, data, maxsize);
	}
	else if (ip->text)
	{
		size = cms_text_read (ip->text, data, maxsize);
	}
	else if (ip->com)
	{
		size = cms_com_read (ip->com, data, maxsize);
	}
	else
	{
		size = cms_round_buffer_read (ip->recv_buffer, data, maxsize);
	}

	// pridame nulu kvuli moznosti prenosu textu
	if (size < 0 && maxsize > 0)
	{
		((char *)data)[0] = 0;
	}
	else if (size >= 0 && size < maxsize)
	{
		((char *)data)[size] = 0;
	}

	// pricteme data
	if (size >= 0 && ip->status_time)
	{
		ip->recv_bytes[0] += size;
		ip->recv_packets[0] += 1;
	}
	
	return size;
}

gpointer cms_ip_client_thread (gpointer data)
{
	cms_ip_t * ip = (cms_ip_t *) ((gpointer *)data)[0];
	cms_client_t * client = (cms_client_t *) ((gpointer *)data)[1];

	((gpointer *)data)[1] = (gpointer) 0;

	while (ip->run)
	{
		if (!cms_client_is_connected (client))
		{
			cms_client_connect (client);
		}
		g_usleep (100000);
	}

	return NULL;
}

void cms_ip_init (cms_ip_t * ip)
{
	GThread * thread;
	gpointer arg[2];
	int i;
	
	if (!ip || ip->magic != CMS_IP_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", ip->magic);
		return;
	}

	if (ip->init)
	{
		return;
	}

	g_mutex_lock (ip->mutex);
	
	if (!ip->init)
	{
		ip->init = TRUE;

		if (ip->client)
		{
			for (i = 0; i < ip->client->len; i++)
			{
				cms_client_init ((cms_client_t *) ip->client->pdata[i]);
			}

			for (i = 0; i < ip->client->len; i++)
			{
				arg[0] = ip;
				arg[1] = ip->client->pdata[i];
				thread = g_thread_create_full (
					cms_ip_client_thread, (gpointer) arg, 0x100000, 
					TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);

				if (thread)
				{
					while (arg[1])
					{
						g_thread_yield ();
					}
					if (!ip->client_thread)
					{
						ip->client_thread = g_ptr_array_new ();
					}
					g_ptr_array_add (ip->client_thread, thread);
				}
				else
				{
					cms_ns_error ("libcms", "g_thread_create_full error");
				}
			}
		}

		if (ip->server)
		{
			for (i = 0; i < ip->server->len; i++)
			{
				cms_server_2_init ((cms_server_2_t *) ip->server->pdata[i]);
			}
		}

		if (ip->udp)
		{
			for (i = 0; i < ip->udp->len; i++)
			{
				cms_udp_init ((cms_udp_t *) ip->udp->pdata[i]);
			}
		}
	}

	g_mutex_unlock (ip->mutex);

	g_mutex_lock (ip_time_mutex);

	if (ip->status_time && !ip_time_thread)
	{
		ip_time_thread = g_thread_create_full (
			cms_ip_time_thread, NULL, 0x100000,
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);

		if (!ip_time_thread)
		{
			cms_ns_error ("libcms", "g_thread_create_full error");
		}
	}

	g_mutex_unlock (ip_time_mutex);
}

void _cms_ip_time_add (cms_ip_t * ip)
{
	// zamkneme zamek
	g_mutex_lock (ip_time_mutex);

	// pridame ip objekt do seznamu
	if (ip->status_time)
	{
		ip_time_list = g_list_append (ip_time_list, ip);
	}

	// odemkneme zamek
	g_mutex_unlock (ip_time_mutex);
}

void _cms_ip_time_remove (cms_ip_t * ip)
{
	// zamkneme zamek
	g_mutex_lock (ip_time_mutex);

	// vyhodime objekt ze seznamu
	if (ip->status_time)
	{
		ip_time_list = g_list_remove (ip_time_list, ip);
	}

	// odemkneme zamek
	g_mutex_unlock (ip_time_mutex);
}

extern char * program_invocation_name;
gpointer cms_ip_time_thread (gpointer data)
{
	cms_udp_t * udp;
	char message[1024];
	char hostname[256];
	char progname[256];
	GTimeVal tv;
	GList * link;
	cms_ip_t * ip;
	unsigned send_bytes;
	unsigned send_packets;
	unsigned recv_bytes;
	unsigned recv_packets;
	unsigned i, maxi;
	int status;
	
	status = gethostname (hostname, sizeof (hostname));
	if (status == -1)
	{
		cms_ns_error ("libcms", "gethostname error");
		return NULL;
	}

	if (g_get_prgname ())
	{
		strcpy (progname, g_get_prgname ());
	}
#ifndef __UCLIBC__
	else
	{
		strcpy (progname, program_invocation_name);
	}
#endif
		
	udp = cms_udp_new (CMS_FLAG_SEND, NULL, 0, "127.0.0.1", "1802", NULL, NULL);
	if (!udp)
	{
		cms_ns_error ("libcms", "cms_udp_new error");
		return NULL;
	}
	
	while (TRUE)
	{
		g_get_current_time (&tv);
		g_usleep (1000000 - tv.tv_usec);

		// zamkneme zamek
		g_mutex_lock (ip_time_mutex);

		// odesleme statistiky
		link = ip_time_list;
		while (link)
		{
			ip = (cms_ip_t *) link->data;
			link = g_list_next (link);
			
			send_bytes = send_packets = 0;
			recv_bytes = recv_packets = 0;
			maxi = ip->status_time - 1;

			for (i = maxi; i > 0; i--)
			{
				send_bytes += ip->send_bytes[i];
				send_packets += ip->send_packets[i];
				recv_bytes += ip->recv_bytes[i];
				recv_packets += ip->recv_packets[i];

				ip->send_bytes[i] = ip->send_bytes[i-1];
				ip->send_packets[i] = ip->send_packets[i-1];
				ip->recv_bytes[i] = ip->recv_bytes[i-1];
				ip->recv_packets[i] = ip->recv_packets[i-1];
			}

			send_bytes += ip->send_bytes[0];
			send_packets += ip->send_packets[0];
			recv_bytes += ip->recv_bytes[0];
			recv_packets += ip->recv_packets[0];

			ip->send_bytes[i] = 0;
			ip->send_packets[i] = 0;
			ip->recv_bytes[i] = 0;
			ip->recv_packets[i] = 0;

			send_bytes = (send_bytes + ip->status_time - 1) / ip->status_time;
			send_packets = (send_packets + ip->status_time - 1) / ip->status_time;
			recv_bytes = (recv_bytes + ip->status_time - 1) / ip->status_time;
			recv_packets = (recv_packets + ip->status_time - 1) / ip->status_time;

			sprintf (message, "%s %s %s %s %s %u %u %u %u",
				hostname, progname, ip->status_name, 
				ip->status_port_1 ? ip->status_port_1 : "0", 
				ip->status_port_2 ? ip->status_port_2 : "0", 
				send_bytes, send_packets, recv_bytes, recv_packets);
			cms_udp_send (udp, message, strlen (message));
		}

		// odemkneme zamek
		g_mutex_unlock (ip_time_mutex);
	}
	
	return NULL;
}

