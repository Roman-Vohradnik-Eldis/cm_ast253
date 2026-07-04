#include "cms_text.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_TEXT_MAGIC 0x72060909

cms_text_t * cms_text_new (const char * path, gboolean write)
{
	cms_text_t * text;

	text = (cms_text_t *) g_malloc0 (sizeof (cms_text_t));
	if (!text)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	text->write = write;

	text->path = g_strdup (path);
	if (!text->path)
	{
		cms_ns_error ("libcms", "g_strdup error");
		goto failed;
	}
	
	text->file = fopen (path, write ? "w" : "r");
	if (!text->file)
	{
		cms_ns_error ("libcms", "open error (%s,%d,%s)", 
			path, errno, strerror (errno));
		goto failed;
	}

#ifdef G_OS_UNIX
	if (write)
	{
		setlinebuf (text->file);
	}
#endif	
	text->magic = CMS_TEXT_MAGIC;
	return text;

failed:
	if (text)
	{
		if (text->file)
		{
			fclose (text->file);
		}
		if (text->path)
		{
			g_free (text->path);
		}
		g_free (text);
	}

	return NULL;
}

cms_text_t * cms_text_new_read (const char * path, unsigned maxsize, unsigned timeout)
{
	cms_text_t * text = cms_text_new (path, FALSE);

	if (text)
	{
		text->maxsize = maxsize ? maxsize : MSGMAX;
		text->timeout = timeout;
	}
	
	return text;
}

cms_text_t * cms_text_new_write (const char * path)
{
	return cms_text_new (path, TRUE);
}

void cms_text_delete (cms_text_t * text)
{
	if (!text || text->magic != CMS_TEXT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", text->magic);
		return;
	}

	text->magic = 0;
	
	if (text->file)
	{
		fclose (text->file);
	}
	if (text->path)
	{
		g_free (text->path);
	}
	g_free (text);
}

void cms_text_close (cms_text_t * text)
{
	if (!text || text->magic != CMS_TEXT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", text->magic);
		return;
	}

	text->magic = 0;
	
	if (text->file)
	{
		fclose (text->file);
		text->file = NULL;
	}
}

int cms_text_read (cms_text_t * text, void * data, unsigned maxsize)
{
	int len;
	
	if (!text || text->magic != CMS_TEXT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", text->magic);
		return 0;
	}

	if (text->write)
	{
		return -1;
	}
	if (!text->file)
	{
		return -1;
	}
	if (text->timeout)
	{
		g_usleep (text->timeout);
	}
	
	memset (data, 0, maxsize);
	if (!fgets ((char *) data, maxsize - 1, text->file))
	{
		return -1;
	}
	
	len = strlen ((char *) data);
	if (((char *)data)[len-1] == '\n')
	{
		((char *)data)[--len] = 0;
	}
	
	return len;
}

int cms_text_write (cms_text_t * text, const void * data, unsigned size)
{
	if (!text || text->magic != CMS_TEXT_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", text->magic);
		return 0;
	}

	if (!text->write)
	{
		return -1;
	}
	if (!text->file)
	{
		return -1;
	}
	
	while (fwrite (data, size, 1, text->file) != 1);
	fputc ('\n', text->file);

	return size;
}

