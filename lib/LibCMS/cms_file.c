#include "cms_file.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_FILE_MAGIC 0x72060905

cms_file_t * cms_file_new (const char * path, gboolean write)
{
	cms_file_t * file;

	file = (cms_file_t *) g_malloc0 (sizeof (cms_file_t));
	if (!file)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	file->fd = -1;
	file->write = write;

	file->path = g_strdup (path);
	if (!file->path)
	{
		cms_ns_error ("libcms", "g_strdup error");
		goto failed;
	}
	
	file->fd = open (path, write ? O_WRONLY | O_CREAT | O_TRUNC : O_RDONLY, 0644);
	if (file->fd == -1)
	{
		cms_ns_error ("libcms", "open error (%s,%d,%s)", 
			path, errno, strerror (errno));
		goto failed;
	}

	file->magic = CMS_FILE_MAGIC;
	return file;

failed:
	if (file)
	{
		if (file->fd != -1)
		{
			close (file->fd);
		}
		if (file->path)
		{
			g_free (file->path);
		}
		g_free (file);
	}

	return NULL;
}

cms_file_t * cms_file_new_read (const char * path, unsigned maxsize, unsigned timeout)
{
	cms_file_t * file = cms_file_new (path, FALSE);

	if (file)
	{
		file->maxsize = maxsize ? maxsize : MSGMAX;
		file->timeout = timeout;
	}
	
	return file;
}

cms_file_t * cms_file_new_write (const char * path)
{
	return cms_file_new (path, TRUE);
}

void cms_file_delete (cms_file_t * file)
{
	if (!file || file->magic != CMS_FILE_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", file->magic);
		return;
	}

	file->magic = 0;
	
	if (file->fd >= 0)
	{
		close (file->fd);
	}
	if (file->path)
	{
		g_free (file->path);
	}
	g_free (file);
}

void cms_file_close (cms_file_t * file)
{
	if (!file || file->magic != CMS_FILE_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", file->magic);
		return;
	}

	file->magic = 0;
	
	if (file->fd >= 0)
	{
		close (file->fd);
		file->fd = -1;
	}
}

int cms_file_read (cms_file_t * file, void * data, unsigned maxsize)
{
	int status;
	
	if (!file || file->magic != CMS_FILE_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", file->magic);
		return 0;
	}

	if (file->write)
	{
		return -1;
	}
	if (file->fd == -1)
	{
		return -1;
	}
	if (file->timeout)
	{
		g_usleep (file->timeout);
	}
	
	status = read (file->fd, data, MIN (maxsize, file->maxsize));
	if (status == 0)
	{
		g_usleep (100000);
		return -1;
	}
	
	return status;
}

int cms_file_write (cms_file_t * file, const void * data, unsigned size)
{
	unsigned count = 0;
	int status;
		
	if (!file || file->magic != CMS_FILE_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", file->magic);
		return 0;
	}

	if (!file->write)
	{
		return -1;
	}
	if (file->fd == -1)
	{
		return -1;
	}

	while (count < size)
	{
		status = write (file->fd, (char *) data + count, size - count);
		if (status > 0)
		{
			count += status;
		}
	}
	
	return size;
}

