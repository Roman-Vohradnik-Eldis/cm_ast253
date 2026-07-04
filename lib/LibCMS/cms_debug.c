#include "cms_debug.h"
#include "cms_misc.h"
#include "cms_server.h"

#define TIME_STAMP_LEN 16
#define COLOR_LEN 10

static char debug_project[32] = { 0 };
static char debug_program[32] = { 0 };
static char debug_ident[64] = { 0 };
static gboolean debug_settings_stdout = TRUE;
static gboolean debug_settings_syslog = FALSE;
static GHashTable * debug_settings_table = NULL;

static cms_debug_settings_t default_debug_settings = {
	CMS_DEBUG_LEVEL_INFO, FALSE, FALSE, FALSE };

static cms_debug_settings_t global_debug_settings = {
	CMS_DEBUG_LEVEL_INFO, FALSE, FALSE, FALSE };

static cms_server_t * debug_server = NULL;
static cms_client_t * debug_client = NULL;
static GThread * debug_server_thread = NULL;
static GHashTable * debug_server_settings_table = NULL;

static GStaticMutex debug_settings_mutex = G_STATIC_MUTEX_INIT;

gpointer _cms_debug_server_thread (gpointer data);

gboolean cms_debug_init (const char * init)
{
	const char * initptr = init;
	char descriptor[1024];
	char * ptr;
	int value;
	
	if (!strcasecmp (init, "yes"))
	{
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_DEBUG);
		cms_warning ("change you debug settings to new format, please");
		return TRUE;
	}
	
	while (sscanf (initptr, "%s", descriptor) == 1)
	{
		initptr = strstr (initptr, descriptor) + strlen (descriptor);


		// rozsekame retezec 
		char ** item = g_strsplit (descriptor, ":", 0);
		if (!item || !item[0])
		{
			cms_ns_error ("libcms", "g_strsplit error: %s", descriptor);
			goto failed;
		}

		// prvni polozka je definice serveru
		if (!strcmp (item[0], "server"))
		{
			if (item[1])
			{
				value = strtol (item[1], &ptr, 0);
				if (*ptr)
				{
					cms_ns_error ("libcms", "strol error: %s", descriptor);
					goto failed;
				}
				cms_debug_server_init (value);
			}
		}

		// prvni polozka je string
		if (g_ascii_isalpha (item[0][0]))
		{
			if (item[1])
			{
				value = strtol (item[1], &ptr, 0);
				if (*ptr)
				{
					cms_ns_error ("libcms", "strol error: %s", descriptor);
					goto failed;
				}
				cms_debug_ns_set_level (item[0], value);

				if (item[2])
				{
					value = strtol (item[2], &ptr, 0);
					if (*ptr)
					{
						cms_ns_error ("libcms", "strol error: %s", descriptor);
						goto failed;
					}
					cms_debug_ns_set_data (item[0], value);

					if (item[3])
					{
						value = strtol (item[3], &ptr, 0);
						if (*ptr)
						{
							cms_ns_error ("libcms", "strol error: %s", descriptor);
							goto failed;
						}
						cms_debug_ns_set_color (item[0], value);

						if (item[4])
						{
							value = strtol (item[4], &ptr, 0);
							if (*ptr)
							{
								cms_ns_error ("libcms", "strol error: %s", descriptor);
								goto failed;
							}
							cms_debug_ns_set_timestamp (item[0], value);
						}
					}
				}
			}
		}

		// prvni polozka je cislo
		else if (g_ascii_isdigit (item[0][0]))
		{
			value = strtol (item[0], &ptr, 0);
			if (*ptr)
			{
				cms_ns_error ("libcms", "strol error: %s", descriptor);
				goto failed;
			}
			cms_debug_ns_set_level (NULL, value);

			if (item[1])
			{
				value = strtol (item[1], &ptr, 0);
				if (*ptr)
				{
					cms_ns_error ("libcms", "strol error: %s", descriptor);
					goto failed;
				}
				cms_debug_ns_set_data (NULL, value);

				if (item[2])
				{
					value = strtol (item[2], &ptr, 0);
					if (*ptr)
					{
						cms_ns_error ("libcms", "strol error: %s", descriptor);
						goto failed;
					}
					cms_debug_ns_set_color (NULL, value);

					if (item[3])
					{
						value = strtol (item[3], &ptr, 0);
						if (*ptr)
						{
							cms_ns_error ("libcms", "strol error: %s", descriptor);
							goto failed;
						}
						cms_debug_ns_set_timestamp (NULL, value);
					}
				}
			}
		}

		else
		{
			cms_ns_error ("libcms", "wrong init: %s", descriptor);
		}

		// uvolnime pamet
		g_strfreev (item);
	}

	return TRUE;

failed:
	return FALSE;
}
 
void cms_debug_server_init (unsigned port)
{
	if (debug_server)
	{
		return;
	}

	char text[64];
	sprintf (text, "%d", port);
	debug_server = cms_server_new (CMS_FLAG_BOTH, "0.0.0.0", text, 10, 
		CMS_TCP_MODE_TEXT, CMS_CLIENT_MAX_BUFFER_SIZE);

	if (debug_server)
	{
		debug_server_thread = g_thread_create_full (
			_cms_debug_server_thread, NULL, 0x100000,
			TRUE, FALSE, G_THREAD_PRIORITY_NORMAL, NULL);
	}
}

gboolean _cms_debug_namespace_init (void)
{
	if (!debug_settings_table)
	{
		debug_settings_table = g_hash_table_new (g_str_hash, g_str_equal);
		if (!debug_settings_table)
		{
			g_print ("Failed creating data structures needed for cms_debug. Using old-style mode.");
			return FALSE;
		}
	}

	if (!debug_server_settings_table)
	{
		debug_server_settings_table = g_hash_table_new (g_str_hash, g_str_equal);
		if (!debug_server_settings_table)
		{
			g_print ("Failed creating data structures needed for cms_debug. Using old-style mode.");
			return FALSE;
		}
	}

	return TRUE;
}

void _cms_hash_table_cleanup (gpointer key, gpointer value, gpointer udata)
{
	g_free (key);
	g_free (value);
}

cms_debug_settings_t * _cms_debug_add_namespace (const gchar * ns)
{
	gpointer data = NULL;

	if (!ns || !ns[0])
	{
		return &global_debug_settings;
	}
	
	if (!_cms_debug_namespace_init ())
	{
		return &global_debug_settings;
	}

	if (g_hash_table_lookup_extended (debug_settings_table, ns, NULL, &data))
	{
		return (cms_debug_settings_t *) data;
	}

	data = g_malloc (sizeof (cms_debug_settings_t));
	gchar * key = g_strdup (ns);
	if (!data || !key)
	{
		return &global_debug_settings;
	}

	memcpy (data, &default_debug_settings, sizeof (cms_debug_settings_t));
	g_hash_table_insert (debug_settings_table, key, data);
	return (cms_debug_settings_t *) data;
}

cms_debug_settings_t * _cms_debug_get_namespace (const gchar * ns)
{
	gpointer data = NULL;
	
	if (!ns || !ns[0])
	{
		return &global_debug_settings;
	}

	if (!_cms_debug_namespace_init ())
	{
		return &global_debug_settings;
	}

	if (g_hash_table_lookup_extended (debug_settings_table, ns, NULL, &data))
	{
		return (cms_debug_settings_t *) data;
	}

	return &global_debug_settings;
}

cms_debug_settings_t * _cms_debug_server_add_namespace (const gchar * ns)
{
	if (!ns)
	{
		ns = "";
	}
	
	gpointer data = NULL;

	if (!_cms_debug_namespace_init ())
	{
		return &global_debug_settings;
	}

	if (g_hash_table_lookup_extended (debug_server_settings_table, ns, NULL, &data))
	{
		return (cms_debug_settings_t *) data;
	}

	data = g_malloc (sizeof (cms_debug_settings_t));
	gchar * key = g_strdup (ns);
	if (!data || !key)
	{
		return &global_debug_settings;
	}

	memset (data, 0, sizeof (cms_debug_settings_t));
	g_hash_table_insert (debug_server_settings_table, key, data);
	return (cms_debug_settings_t *) data;
}

void cms_debug_set_project (const char * project)
{
	g_strlcpy (debug_project, project, sizeof (debug_project));
	g_snprintf (debug_ident, sizeof (debug_ident), 
		"%s.%s", debug_project, debug_program);
}

void cms_debug_set_program (const char * program)
{
	g_strlcpy (debug_program, program, sizeof (debug_program));
	g_snprintf (debug_ident, sizeof (debug_ident), 
		"%s.%s", debug_project, debug_program);
}

void cms_debug_done ()
{
	if (debug_settings_table)
	{
		g_hash_table_foreach (debug_settings_table, (GHFunc) _cms_hash_table_cleanup, NULL);
		g_hash_table_destroy (debug_settings_table);
		debug_settings_table = NULL;
	}

	if (debug_server_settings_table)
	{
		g_hash_table_foreach (debug_server_settings_table, (GHFunc) _cms_hash_table_cleanup, NULL);
		g_hash_table_destroy (debug_server_settings_table);
		debug_server_settings_table = NULL;
	}
}

void cms_debug_set_stdout (gboolean stdout)
{
	debug_settings_stdout = stdout;
}

void cms_debug_set_syslog (gboolean syslog)
{
#ifdef G_OS_UNIX
	if (debug_settings_syslog != syslog)
	{
		if (syslog)
		{
			openlog (debug_ident, LOG_CONS | LOG_NDELAY | LOG_PID, LOG_USER);
		}
		else
		{
			closelog ();
		}
	}
#endif

	debug_settings_syslog = syslog;
}

gboolean cms_debug_get_stdout ()
{
	return debug_settings_stdout;
}

gboolean cms_debug_get_syslog ()
{
	return debug_settings_syslog;
}

void cms_debug_ns_set_level (const gchar * ns, unsigned level)
{
	g_static_mutex_lock (&debug_settings_mutex);
	
	cms_debug_settings_t * settings = _cms_debug_add_namespace (ns);
	settings->level = level;

	g_static_mutex_unlock (&debug_settings_mutex);
}

void cms_debug_ns_set_data (const gchar * ns, gboolean data)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_add_namespace (ns);
	settings->data = data;

	g_static_mutex_unlock (&debug_settings_mutex);
}

void cms_debug_ns_set_color (const gchar * ns, gboolean color)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_add_namespace (ns);
	settings->color = color;

	g_static_mutex_unlock (&debug_settings_mutex);
}

void cms_debug_ns_set_timestamp (const gchar * ns, gboolean timestamp)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_add_namespace (ns);
	settings->timestamp = timestamp;

	g_static_mutex_unlock (&debug_settings_mutex);
}

unsigned cms_debug_ns_get_level (const gchar * ns)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_get_namespace (ns);
	unsigned level = settings->level;

	g_static_mutex_unlock (&debug_settings_mutex);

	return level;
}

unsigned cms_debug_server_get_level (const gchar * ns)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_server_add_namespace (ns);
	unsigned level = settings->level;

	g_static_mutex_unlock (&debug_settings_mutex);

	return level;
}

unsigned cms_debug_max_get_level (const gchar * ns)
{
	unsigned level = cms_debug_ns_get_level (ns);
	unsigned server_level = cms_debug_server_get_level (ns);
	return MAX (level, server_level);
}

gboolean cms_debug_ns_get_data (const gchar * ns)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_get_namespace (ns);
	gboolean data = settings->data;

	g_static_mutex_unlock (&debug_settings_mutex);

	return data;
}

gboolean cms_debug_server_get_data (const gchar * ns)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_server_add_namespace (ns);
	gboolean data = settings->data;

	g_static_mutex_unlock (&debug_settings_mutex);

	return data;
}

gboolean cms_debug_max_get_data (const gchar * ns)
{
	gboolean data = cms_debug_ns_get_data (ns);
	gboolean server_data = cms_debug_server_get_data (ns);
	return (data | server_data);
}

gboolean cms_debug_ns_get_color (const gchar * ns)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_get_namespace (ns);
	gboolean color = settings->color;

	g_static_mutex_unlock (&debug_settings_mutex);

	return color;
}

gboolean cms_debug_ns_get_timestamp (const gchar * ns)
{
	g_static_mutex_lock (&debug_settings_mutex);

	cms_debug_settings_t * settings = _cms_debug_get_namespace (ns);
	gboolean timestamp = settings->timestamp;

	g_static_mutex_unlock (&debug_settings_mutex);

	return timestamp;
}

void _cms_debug_get_time_stamp (char * timestamp, int len)
{
	GTimeVal Time;
	unsigned hh;
	unsigned mm;
	unsigned ss;
	unsigned msec;
	double time_sec;
	int t_msec;

	g_get_current_time (&Time);

	time_sec = (double) (Time.tv_sec % (24 * 3600)) + 1e-6 * Time.tv_usec;
	t_msec   = (int) (time_sec * 1000.);

	msec  = t_msec % 1000;
	t_msec /= 1000;

	hh = t_msec / (3600);
	mm = (t_msec - hh * 3600) / 60;
	ss = (t_msec - hh * 3600 - mm * 60);

	g_snprintf (timestamp, len, "[%02d:%02d:%02d.%03d] ",hh,mm,ss,msec);
}

void cms_debug_print_text (char * out, unsigned outsize, 
	const char * file, const char * function, int line, 
	const char * ns, unsigned level, const char * text, ...)
{
	char text_message[BUFMAX];
	char text_color_b[10] = { 0 };
	char text_color_e[10] = { 0 };
	char text_timestamp[TIME_STAMP_LEN] = { 0 };
	char * text_label = NULL;
	va_list ap;
	int pid = -1;
	int i;

	gboolean ns_level = (cms_debug_ns_get_level (ns) >= level);
	gboolean server_level = (cms_debug_server_get_level (ns) >= level);
	
	if (!ns_level && !server_level)
	{
		return;
	}
	
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

	switch (level)
	{
		case CMS_DEBUG_LEVEL_ERROR:
		{
			text_label = "ERROR";
			break;
		}
		case CMS_DEBUG_LEVEL_WARNING:
		{
			text_label = "WARNING";
			break;
		}
		case CMS_DEBUG_LEVEL_INFO:
		{
			text_label = "INFO";
			break;
		}
		default:
		{
			text_label = "DEBUG";
		}
	}
	
	if (cms_debug_ns_get_color (ns))
	{
		g_strlcpy (text_color_b, "\e[1;31m\0", COLOR_LEN);
		g_strlcpy (text_color_e, "\e[0m\0", COLOR_LEN);
	}

	if (cms_debug_ns_get_timestamp (ns))
	{
		_cms_debug_get_time_stamp (text_timestamp, TIME_STAMP_LEN);
	}

	va_start (ap, text);
	g_vsnprintf (text_message, sizeof (text_message), text, ap);
	va_end (ap);

	if (ns_level && out)
	{
		g_snprintf (out, outsize, "%s (%d,%s,%s,%d) %s", 
			text_label, pid, file, function, line, text_message);
	}

	if (server_level && debug_client 
		&& cms_client_is_connected (debug_client))
	{
		char buffer[MSGMAX];
		g_snprintf (buffer, sizeof (buffer), "%s (%d,%s,%s,%d) %s", 
			text_label, pid, file, function, line, text_message);
		cms_client_send (debug_client, buffer, strlen (buffer));
	}
		
	if (ns_level && debug_settings_stdout)
	{
		g_print ("%s%s %s%s%s (%d,%s,%s,%d) %s\n", 
			text_timestamp, debug_program,
			text_color_b, text_label, text_color_e, 
			pid, file, function, line, text_message);
	}

#ifdef G_OS_UNIX
	if (ns_level && debug_settings_syslog)
	{
		char buffer[MSGMAX];
		int len;
		
		g_snprintf (buffer, sizeof (buffer), "%s (%d,%s,%s,%d) %s", 
			text_label, pid, file, function, line, text_message);
		len = strlen (buffer);
		
		for (i = 0; i < len; i += 500)
		{
			char buffer2[512];
			g_strlcpy (buffer2, buffer + i, 501);
			syslog (LOG_DEBUG, "%s", buffer2);
		}
	}
#endif
}

void cms_debug_print_binary (char * out, unsigned outsize, const char * file,
	const char * function, int line, const char * ns, int length,
	const void * buffer)
{
	const char * data = (const char *) buffer;
	char text_message[BUFMAX];
	char text_color_b[10] = { 0 };
	char text_color_e[10] = { 0 };
	char text_timestamp[TIME_STAMP_LEN] = { 0 };
	int pid = -1;
	int i, len;

	gboolean ns_data = cms_debug_ns_get_data (ns);
	gboolean server_data = cms_debug_server_get_data (ns);

	if (!ns_data && !server_data)
	{
		return;
	}

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

	if (cms_debug_ns_get_color (ns))
	{
		g_strlcpy (text_color_b, "\e[1;31m\0", COLOR_LEN);
		g_strlcpy (text_color_e, "\e[0m\0", COLOR_LEN);
	}

	if (cms_debug_ns_get_timestamp (ns))
	{
		_cms_debug_get_time_stamp (text_timestamp, TIME_STAMP_LEN);
	}

	len = 0;
	for (i = 0; i < length; i++)
	{
		if (len + 3 >= sizeof (text_message))
		{
			break;
		}
		
		g_snprintf (text_message + len, sizeof (text_message) - len, "%02hhx ", data[i]);
		len = g_utf8_strlen (text_message, sizeof (text_message));
	}

	if (ns_data && out)
	{
		g_snprintf (out, outsize, "DATA (%d,%s,%s,%d) %s", 
			pid, file, function, line, text_message);
	}

	if (server_data && debug_client 
		&& cms_client_is_connected (debug_client))
	{
		char buffer[MSGMAX];
		g_snprintf (buffer, sizeof (buffer), "DATA (%d,%s,%s,%d) %s", 
			pid, file, function, line, text_message);
		cms_client_send (debug_client, buffer, strlen (buffer));
	}
		
	if (ns_data && debug_settings_stdout)
	{
		g_print ("%s%s %sDATA%s (%d,%s,%s,%d) %s\n", 
			text_timestamp, debug_program,
			text_color_b, text_color_e, 
			pid, file, function, line, text_message);
	}

#ifdef G_OS_UNIX
	if (ns_data && debug_settings_syslog)
	{
		char buffer[MSGMAX];
		int len;
		
		g_snprintf (buffer, sizeof (buffer), "DATA (%d,%s,%s,%d) %s", 
			pid, file, function, line, text_message);
		len = strlen (buffer);
		
		for (i = 0; i < len; i += 500)
		{
			char buffer2[512];
			g_strlcpy (buffer2, buffer + i, 501);
			syslog (LOG_DEBUG, "%s", buffer2);
		}
	}
#endif
}

gpointer _cms_debug_server_thread (gpointer arg)
{
	cms_debug_settings_t * settings;
	cms_client_t * new_client;
	cms_client_t * old_client;
	char buffer[1024];
	char ns[1024];
	unsigned level;
	unsigned data;
	int length;
	int status;
	GList * list;
	GList * link;
	
	while (TRUE)
	{
		if (!debug_client || !cms_client_is_connected (debug_client))
		{
			if (!(new_client = cms_server_wait_for_client (debug_server)))
			{
				sleep (1);
				continue;
			}

			old_client = debug_client;
			debug_client = new_client;

			g_static_mutex_lock (&debug_settings_mutex);
			cms_client_send (debug_client, debug_ident, strlen (debug_ident));
			list = g_hash_table_get_keys (debug_server_settings_table);
			for (link = list; link; link = g_list_next (link))
			{
				char * ns = (char *) link->data;
				cms_client_send (debug_client, ns, strlen (ns));
			}
			g_list_free (list);
			g_static_mutex_unlock (&debug_settings_mutex);

			if (old_client)
			{
				cms_client_delete (old_client);
			}			
		}

		length = cms_client_recv (debug_client, buffer, sizeof (buffer));
		if (length < 0)
		{
			cms_client_disconnect (debug_client);
			g_static_mutex_lock (&debug_settings_mutex);
			list = g_hash_table_get_values (debug_server_settings_table);
			for (link = list; link; link = g_list_next (link))
			{
				settings = (cms_debug_settings_t *) link->data;
				settings->level = 0;
				settings->data = FALSE;
			}
			g_list_free (list);
			g_static_mutex_unlock (&debug_settings_mutex);
			continue;
		}

		buffer[length] = 0;
		status = sscanf (buffer, "%u %u", &level, &data);
		if (status == 2)
		{
			g_static_mutex_lock (&debug_settings_mutex);
			settings = _cms_debug_server_add_namespace ("");
			settings->level = level;
			settings->data = data;
			g_static_mutex_unlock (&debug_settings_mutex);
			continue;
		}
		else if (status == 1)
		{
			g_static_mutex_lock (&debug_settings_mutex);
			settings = _cms_debug_server_add_namespace ("");
			settings->level = level;
			settings->data = FALSE;
			g_static_mutex_unlock (&debug_settings_mutex);
			continue;
		}

		status = sscanf (buffer, "%s %u %u", ns, &level, &data);
		if (status == 3)
		{
			g_static_mutex_lock (&debug_settings_mutex);
			settings = _cms_debug_server_add_namespace (ns);
			settings->level = level;
			settings->data = data;
			g_static_mutex_unlock (&debug_settings_mutex);
			continue;
		}
		else if (status == 2)
		{
			g_static_mutex_lock (&debug_settings_mutex);
			settings = _cms_debug_server_add_namespace (ns);
			settings->level = level;
			settings->data = FALSE;
			g_static_mutex_unlock (&debug_settings_mutex);
			continue;
		}
	}
	
	return NULL;
}

