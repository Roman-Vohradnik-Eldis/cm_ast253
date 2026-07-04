#include "cms_misc.h"
#include "cms_debug.h"

unsigned short ipchecksum (const void * addr, int count)
{
	register unsigned short * ptr = (unsigned short *) addr;
	register long sum = 0;
	while (count > 1)
	{
		/*  This is the inner loop */
		sum += *ptr;
		ptr++;
		count -= 2;
	}
	/*  Add left-over byte, if any */
	if (count > 0)
	{
		sum += *((unsigned char *) ptr);
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return ~sum;
}

gboolean time2ymdhms (time_t time, unsigned * year, unsigned * month, 
	unsigned * day, unsigned * hour, unsigned * min, unsigned * sec)
{
	struct tm tm;

	if (!gmtime_r (&time, &tm))
	{
		return FALSE;
	}

	if (year) *year = tm.tm_year + 1900;
	if (month) *month = tm.tm_mon + 1;
	if (day) *day = tm.tm_mday;
	if (hour) *hour = tm.tm_hour;
	if (min) *min = tm.tm_min;
	if (sec) *sec = tm.tm_sec;
	return TRUE;
}

gboolean localtime2ymdhms (time_t time, unsigned * year, unsigned * month, 
	unsigned * day, unsigned * hour, unsigned * min, unsigned * sec)
{
	struct tm tm;

	if (!localtime_r (&time, &tm))
	{
		return FALSE;
	}

	if (year) *year = tm.tm_year + 1900;
	if (month) *month = tm.tm_mon + 1;
	if (day) *day = tm.tm_mday;
	if (hour) *hour = tm.tm_hour;
	if (min) *min = tm.tm_min;
	if (sec) *sec = tm.tm_sec;
	return TRUE;
}

gboolean ymdhms2time (unsigned year, unsigned month, unsigned day, 
	unsigned hour, unsigned min, unsigned sec, time_t * time)
{
	struct tm tm;
	time_t tmp;

	memset (&tm, 0, sizeof (tm));

	tm.tm_sec = sec;
	tm.tm_min = min;
	tm.tm_hour = hour;
	tm.tm_mday = day;
	tm.tm_mon = month - 1;
	tm.tm_year = year - 1900;
	tm.tm_isdst = -1;

	tmp = timegm (&tm);
	if (tmp == -1)
	{
		if (time)
		{
			*time = 0;
		}
		return FALSE;
	}

	if (time)
	{
		*time = tmp;
	}
	return TRUE;
}

gboolean ymdhms2localtime (unsigned year, unsigned month, unsigned day, 
	unsigned hour, unsigned min, unsigned sec, time_t * time)
{
	struct tm tm;
	time_t tmp;

	memset (&tm, 0, sizeof (tm));

	tm.tm_sec = sec;
	tm.tm_min = min;
	tm.tm_hour = hour;
	tm.tm_mday = day;
	tm.tm_mon = month - 1;
	tm.tm_year = year - 1900;
	tm.tm_isdst = -1;

	tmp = timelocal (&tm);
	if (tmp == -1)
	{
		if (time)
		{
			*time = 0;
		}
		return FALSE;
	}

	if (time)
	{
		*time = tmp;
	}
	return TRUE;
}

gboolean time2text (time_t time, char * text)
{
	unsigned year, month, day, hour, min, sec;
	gboolean status;

	status = time2ymdhms (time, &year, &month, &day, &hour, &min, &sec);
	sprintf (text, "%04u/%02u/%02u %02u:%02u:%02u", year, month, day, hour, min, sec);

	return status;
}

gboolean time2texts (time_t time, char * text_date, char * text_time)
{
	unsigned year, month, day, hour, min, sec;
	gboolean status;

	status = time2ymdhms (time, &year, &month, &day, &hour, &min, &sec);

	if (text_date)
	{
		sprintf (text_date, "%04u/%02u/%02u", year, month, day);
	}
	if (text_time)
	{
		sprintf (text_time, "%02u:%02u:%02u", hour, min, sec);
	}

	return status;
}

gboolean text2time (const char * text, time_t * time)
{
	unsigned year, month, day, hour, min, sec;
	int status;

	status = sscanf (text, "%u/%u/%u %u:%u:%u", &year, &month, &day, &hour, &min, &sec);
	if (status != 6)
	{
		return FALSE;
	}

	return ymdhms2time (year, month, day, hour, min, sec, time);
}

gboolean texts2time (const char * text_date, const char * text_time, time_t * time)
{
	unsigned year, month, day, hour, min, sec;
	int status;

	if (text_date)
	{
		status = sscanf (text_date, "%u/%u/%u", &year, &month, &day);
		if (status != 3)
		{
			return FALSE;
		}
	}
	else
	{
		year = 1970;
		month = 1;
		day = 1;
	}

	if (text_time)
	{
		status = sscanf (text_time, "%u:%u:%u", &hour, &min, &sec);
		if (status != 3)
		{
			return FALSE;
		}
	}
	else
	{
		hour = 0;
		min = 0;
		sec = 0;
	}

	return ymdhms2time (year, month, day, hour, min, sec, time);
}

FILE * fopen_case (const char * filename, const char * mode)
{
	char path[1024];
	GDir * dir = NULL;
	const gchar * name;
	gchar ** strarr;
	gchar ** strarr_orig = NULL;

	// nepodarilo se, prohledame adresarovou strukturu
	strarr_orig = strarr = g_strsplit (filename, "/", 0);

	// neco je spatne
	if (!strarr || !strarr[0])
	{
		goto failed;
	}

	// je zadan pouze soubor bez cesty
	if (!strarr[1])
	{
		dir = g_dir_open (".", 0, NULL);
		g_strlcpy (path, "", sizeof (path));
	}

	// je zadana cesta
	else
	{
		// cesta je absolutni
		if (strarr[0][0] == 0)
		{
			dir = g_dir_open ("/", 0, NULL);
			g_strlcpy (path, "", sizeof (path));
		}

		// cesta je relativni
		else
		{
			dir = g_dir_open (strarr[0], 0, NULL);
			g_strlcpy (path, strarr[0], sizeof (path));
		}
		strarr++;
	}

	// nepodarilo se otevrit adresar
	if (!dir)
	{
		goto failed;
	}
	while ((name = g_dir_read_name (dir)))
	{
		if (!g_ascii_strcasecmp (name, strarr[0]))
		{
			// pridame soubor k ceste
			g_strlcat (path, "/", sizeof (path));
			g_strlcat (path, name, sizeof (path));

			// zavreme otevreny adresar
			g_dir_close (dir);

			// jsme na konci, mame celou cestu, koncime
			if (!(++strarr)[0])
			{
				g_strfreev (strarr_orig);
				return fopen (path, mode);
			}

			// otevreme adresar
			dir = g_dir_open (path, 0, NULL);
			if (!dir)
			{
				goto failed;
			}
		}
	}

failed:
	if (dir)
	{
		g_dir_close (dir);
	}

	g_strfreev (strarr_orig);
	return NULL;
}

ssize_t read_all (int fd, void * buffer, size_t count)
{
	ssize_t status;
	ssize_t size = 0;

	while (size < count)
	{
		status = read (fd, (char *) buffer + size, count - size);
		if (status == -1)
		{
			return -1;
		}
		if (status == 0)
		{
			return size;
		}
		size += status;
	}

	return size;
}

ssize_t write_all (int fd, const void * buffer, size_t count)
{
	ssize_t status;
	ssize_t size = 0;

	while (size < count)
	{
		status = write (fd, (const char *) buffer + size, count - size);
		if (status == -1)
		{
			return -1;
		}
		size += status;
	}

	return size;
}

int cms_run_command (const char * text, ...)
{
	char command[4096];
	va_list ap;
	int status;

	// vygenerujeme prikaz
	va_start (ap, text);
	vsnprintf (command, sizeof (command), text, ap);
	va_end (ap);

	cms_ns_debug ("libcms", "COMMAND: %s", command);

	// pridame presmerovani
	if (!cms_debug_get_stdout () 
		|| cms_debug_ns_get_level ("libcms") < CMS_DEBUG_LEVEL_DEBUG)
	{
		if (!strchr (command, '<'))
		{
			strcat (command, " </dev/null");
		}
		if (!strchr (command, '>'))
		{
			strcat (command, " >/dev/null 2>/dev/null");
		}
	}

	// spustime prikaz
	status = system (command);

	// vratime navratovou hodnotu
#ifdef G_OS_UNIX
	if (status == -1)
	{
		cms_ns_error ("libcms", "system error (%s,%d,%s)", 
			command, errno, strerror (errno));
		return -1;
	}

	status = WEXITSTATUS (status);

	if (status == -1)
	{
		cms_ns_error ("libcms", "system exit status error (%s,%d,%s)", 
			command, errno, strerror (errno));
		return -1;
	}

	return status;
#else
	return status;
#endif
}

