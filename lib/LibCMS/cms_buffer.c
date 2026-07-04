#include "cms_buffer.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_ROUND_BUFFER_MAGIC 0x72060901
#define CMS_ROUND_BUFFER_DEFAULT_SIZE_1 0x1000
#define CMS_ROUND_BUFFER_DEFAULT_SIZE_2 0x10000

cms_round_buffer_t * cms_round_buffer_new (const char * name, 
	unsigned maxsize, int flags)
{
	cms_round_buffer_t * buffer = g_malloc0 (sizeof (cms_round_buffer_t));
	if (!buffer)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}

	buffer->name = g_strdup (name);
	if (!buffer->name)
	{
		cms_ns_error ("libcms", "g_strdup error");
		goto failed;
	}
	
	buffer->data = g_malloc (CMS_ROUND_BUFFER_DEFAULT_SIZE_1);
	if (!buffer->data)
	{
		cms_ns_error ("libcms", "g_malloc error");
		goto failed;
	}

#ifdef __UCLIBC__
	if (maxsize > 0x10000)
	{
		maxsize = 0x10000;
	}
#endif

	buffer->type = CMS_ROUND_BUFFER_TYPE_RAW;
	buffer->max_buffer_size = MAX (maxsize, CMS_ROUND_BUFFER_DEFAULT_SIZE_1);
	buffer->max_buffer_time = 0;
	buffer->buffer_size = CMS_ROUND_BUFFER_DEFAULT_SIZE_1;
	buffer->free_space = CMS_ROUND_BUFFER_DEFAULT_SIZE_1;
	buffer->free_pos = 0;
	buffer->fill_pos = 0;
	buffer->messages = 0;
	
	buffer->mutex = g_mutex_new ();
	if (!buffer->mutex)
	{
		cms_ns_error ("libcms", "g_mutex_new error");
		goto failed;
	}
	
	buffer->cond = g_cond_new ();
	if (!buffer->cond)
	{
		cms_ns_error ("libcms", "g_cond_new error");
		goto failed;
	}
	
	g_get_current_time (&buffer->check_time);
	buffer->max_fill_space = CMS_ROUND_BUFFER_DEFAULT_SIZE_1;

	buffer->magic = CMS_ROUND_BUFFER_MAGIC;
	cms_round_buffer_set_flags (buffer, flags);
	return buffer;

failed:
	if (buffer)
	{
		if (buffer->cond)
		{
			g_cond_free (buffer->cond);
		}
		if (buffer->mutex)
		{
			g_mutex_free (buffer->mutex);
		}
		if (buffer->data)
		{
			g_free (buffer->data);
		}
		if (buffer->name)
		{
			g_free (buffer->name);
		}
		g_free (buffer);
	}

	return NULL;
}

cms_round_buffer_t * cms_round_buffer_new_with_block (const char * name, 
	unsigned maxsize, int flags)
{
	cms_round_buffer_t * buffer = cms_round_buffer_new (name, maxsize, flags);
	buffer->type = CMS_ROUND_BUFFER_TYPE_BLOCK;
	return buffer;
}

cms_round_buffer_t * cms_round_buffer_new_with_time_block (const char * name, 
	unsigned maxsize, unsigned maxtime, int flags)
{
	cms_round_buffer_t * buffer = cms_round_buffer_new (name, maxsize, flags);
	buffer->type = CMS_ROUND_BUFFER_TYPE_TIME_BLOCK;
	buffer->max_buffer_time = maxtime;
	return buffer;
}

void cms_round_buffer_delete (cms_round_buffer_t * buffer)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return;
	}

	buffer->magic = 0;

	if (buffer->mutex)
	{
		g_mutex_free (buffer->mutex);
	}
	if (buffer->cond)
	{
		g_cond_free (buffer->cond);
	}
	if (buffer->data)
	{
		g_free (buffer->data);
	}
	if (buffer->name)
	{
		g_free (buffer->name);
	}
	g_free (buffer);
}

gboolean cms_round_buffer_is_empty (cms_round_buffer_t * buffer)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return FALSE;
	}

	return (buffer->free_space == buffer->buffer_size);
}

void _cms_round_buffer_clear (cms_round_buffer_t * buffer)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return;
	}

	buffer->free_pos = 0;
	buffer->fill_pos = 0;
	buffer->free_space = buffer->buffer_size;
}

void cms_round_buffer_clear (cms_round_buffer_t * buffer)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return;
	}

	g_mutex_lock (buffer->mutex);
	_cms_round_buffer_clear (buffer);
	g_mutex_unlock (buffer->mutex);
}

unsigned cms_round_buffer_get_fill_space (cms_round_buffer_t * buffer)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return 0;
	}

	return buffer->buffer_size - buffer->free_space;
}

unsigned cms_round_buffer_get_messages (cms_round_buffer_t * buffer)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return 0;
	}

	return buffer->messages;
}

void cms_round_buffer_set_flags (cms_round_buffer_t * buffer, int flags)
{
	if (!buffer || buffer->magic != CMS_ROUND_BUFFER_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", buffer->magic);
		return;
	}

	g_mutex_lock (buffer->mutex);

	buffer->flags = flags;

	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);
}

// private function, neprovadim lock
void _cms_round_buffer_set_size (cms_round_buffer_t * buffer, unsigned size)
{
	unsigned char * newbuffer;
	
	cms_ns_debug ("libcms", "changing buf size (%s) %d -> %d", 
		buffer->name, buffer->buffer_size, size);

	if ((buffer->buffer_size - buffer->free_space) > size)
	{
		cms_ns_error ("libcms", "unable to reduce buffer size (%s)", buffer->name);
		return;
	}

	// pokusim se akokovat novy buf
	newbuffer = g_malloc (size);
	if (!newbuffer)
	{
		cms_ns_error ("libcms", "unable to allocate buffer (%s)", buffer->name);
		_cms_round_buffer_clear (buffer);
		return;
	}

	// volne misto je na zacatku a na konci kruhoveho bufferu
	if (buffer->fill_pos < buffer->free_pos)
	{
		memcpy (newbuffer, buffer->data + buffer->fill_pos, 
			buffer->free_pos - buffer->fill_pos);
	}

	// volne misto je uprostred kruhoveho buffer,
	// popripade je kruhovy buffer zaplneny cely
	else if (buffer->free_space < buffer->buffer_size)
	{
		memcpy (newbuffer, buffer->data + buffer->fill_pos, 
			buffer->buffer_size - buffer->fill_pos);
		memcpy (newbuffer + buffer->buffer_size - buffer->fill_pos, 
			buffer->data, buffer->free_pos);
	}

	// zrusime puvodni buffer
	g_free (buffer->data);
	buffer->data = newbuffer;

	// aktualizujeme udaje o buffer
	buffer->fill_pos = 0;
	buffer->free_pos = buffer->buffer_size - buffer->free_space;
	buffer->free_space = buffer->free_space - buffer->buffer_size + size;
	buffer->buffer_size = size;
}

void _cms_round_buffer_check_size (cms_round_buffer_t * buffer)
{
	GTimeVal tv;

	if (!(buffer->flags & CMS_ROUND_BUFFER_WRITE_GROW))
	{
		return;
	}

	g_get_current_time (&tv);
	if (buffer->check_time.tv_sec > tv.tv_sec)
	{
		return;
	}

	if (buffer->max_fill_space < buffer->buffer_size / 4
		&& buffer->buffer_size > CMS_ROUND_BUFFER_DEFAULT_SIZE_2)
	{
		_cms_round_buffer_set_size (buffer, buffer->buffer_size / 2);
	}
	
	buffer->check_time.tv_sec = tv.tv_sec + 60;
	buffer->max_fill_space = MAX (buffer->buffer_size - buffer->free_space, 
		CMS_ROUND_BUFFER_DEFAULT_SIZE_1);
}

int __cms_round_buffer_read_raw (cms_round_buffer_t * buffer, 
	void * data, unsigned size)
{
	unsigned size1, size2;	

	// zjistime skutecnou delku dat
	if (buffer->buffer_size == buffer->free_space)
	{
		return -1;
	}
	
	if ((buffer->buffer_size - buffer->free_space) < size)
	{
		size = buffer->buffer_size - buffer->free_space;
	}
	
	// zkopirujeme data z bufferu
	if (data)
	{
		if (buffer->fill_pos + size > buffer->buffer_size)
		{
			size2 = buffer->fill_pos + size - buffer->buffer_size;
			size1 = size - size2;
			memcpy (data, buffer->data + buffer->fill_pos, size1);
			memcpy ((char *) data + size1, buffer->data, size2);
		}
		else
		{
			memcpy (data, buffer->data + buffer->fill_pos, size);
		}
	}

	// modifikujeme promenne
	buffer->fill_pos = (buffer->fill_pos + size) % buffer->buffer_size;
	buffer->free_space += size;
	buffer->messages -= size;
	if (buffer->free_space > buffer->buffer_size)
	{
		cms_ns_error ("libcms", "%s: buffer_size %d, free_pos %d, fill_pos %d, free_space %d", 
			buffer->name, buffer->buffer_size, buffer->free_pos, 
			buffer->fill_pos, buffer->free_space);
		return -1;
	}

	return size;
}

int _cms_round_buffer_read_raw (cms_round_buffer_t * buffer, 
	void * data, unsigned size)
{
	int retval = 0;

	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// v bufferu neni dostatek dat, cekame
	while (((buffer->flags & CMS_ROUND_BUFFER_READ_BLOCK_ALL) 
		&& ((buffer->buffer_size - buffer->free_space) < size))
		|| ((buffer->flags & CMS_ROUND_BUFFER_READ_BLOCK) 
		&& (buffer->buffer_size == buffer->free_space)))
	{
		g_cond_wait (buffer->cond, buffer->mutex);
	}

	// cteme z bufferu
	retval = __cms_round_buffer_read_raw (buffer, data, size);

	// zkontrolujeme vyuziti bufferu
	_cms_round_buffer_check_size (buffer);

	// odemkneme pristup k bufferu
	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);

	return retval;
}


int __cms_round_buffer_write_raw (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{
	unsigned size1, size2;

	// zkopirujeme do bufferu data
	if (buffer->free_pos + size > buffer->buffer_size)
	{
		size2 = buffer->free_pos + size - buffer->buffer_size;
		size1 = size - size2;
		memcpy (buffer->data + buffer->free_pos, data, size1);
		memcpy (buffer->data, (char *) data + size1, size2);
	}
	else
	{
		memcpy (buffer->data + buffer->free_pos, data, size);
	}

	// modifikujeme promenne
	buffer->free_pos = (buffer->free_pos + size) % buffer->buffer_size;
	buffer->free_space -= size;
	buffer->messages += size;

	return size;
}

int _cms_round_buffer_write_raw (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{
	int retval = 0;
	
	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// v bufferu neni dost mista
	while (size > buffer->free_space)
	{
		// zvetsime velikost bufferu
		if ((buffer->flags & CMS_ROUND_BUFFER_WRITE_GROW)
			&& buffer->buffer_size < buffer->max_buffer_size)
		{
			_cms_round_buffer_set_size (buffer, 
				buffer->max_buffer_size < buffer->buffer_size * 2 
				? buffer->max_buffer_size : buffer->buffer_size * 2);
		}
		// vyhodime data za zacatku bufferu
		else if ((buffer->flags & CMS_ROUND_BUFFER_WRITE_THROW)
			&& buffer->free_space < buffer->buffer_size)
		{
			cms_ns_warning ("libcms", "throwing data (%s)", buffer->name);
			__cms_round_buffer_read_raw (buffer, NULL, size - buffer->free_space);
		}
		// pockame na uvolneni bufferu
		else if (buffer->flags & CMS_ROUND_BUFFER_WRITE_BLOCK)
		{
			if (size > buffer->buffer_size)
			{
				cms_ns_error ("libcms", "small buffer (%s)", buffer->name);
				goto end;
			}
			g_cond_wait (buffer->cond, buffer->mutex);
		}
		// zahodime data
		else
		{
			cms_ns_error ("libcms", "not enough space in buffer (%s)", buffer->name);
			goto end;
		}
	}

	// zapiseme do bufferu
	retval = __cms_round_buffer_write_raw (buffer, data, size);
	if (retval != size)
	{
		cms_ns_error ("libcms", "__cms_round_buffer_write_raw error");
	}
			
	// zapamatujeme si obsazeni bufferu
	buffer->max_fill_space = MAX (buffer->max_fill_space, 
		buffer->buffer_size - buffer->free_space);

end:
	// odemkneme pristup k bufferu a posleme signal
	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);

	return retval;
}

int __cms_round_buffer_read_block (cms_round_buffer_t * buffer, 
	void * data, unsigned maxsize)
{
	unsigned size1, size2, size;
	
	// zjistime delku dat v bufferu
	size = buffer->data[buffer->fill_pos];
	buffer->fill_pos = (buffer->fill_pos + 1) % buffer->buffer_size;
	size |= ((unsigned) buffer->data[buffer->fill_pos]) << 8;
	buffer->fill_pos = (buffer->fill_pos + 1) % buffer->buffer_size;
	size |= ((unsigned) buffer->data[buffer->fill_pos]) << 16;
	buffer->fill_pos = (buffer->fill_pos + 1) % buffer->buffer_size;
	size |= ((unsigned) buffer->data[buffer->fill_pos]) << 24;
	buffer->fill_pos = (buffer->fill_pos + 1) % buffer->buffer_size;
	buffer->free_space += 4;

	if (size > maxsize)
	{
		cms_ns_error ("libcms", "%s: size %u, maxsize %u, buffer->buffer_size %u, "
			"buffer->free_space %u, buffer->free_pos %u, buffer->fill_pos %u",
			buffer->name, size, maxsize, buffer->buffer_size, buffer->free_space, 
			buffer->free_pos, buffer->fill_pos, buffer->data);
		_cms_round_buffer_clear (buffer);
		return -1;
	}

	// zkopirujeme data z bufferu
	if (data)
	{
		if (buffer->fill_pos + size > buffer->buffer_size)
		{
			size2 = buffer->fill_pos + size - buffer->buffer_size;
			size1 = size - size2;
			memcpy (data, buffer->data + buffer->fill_pos, size1);
			memcpy ((char *) data + size1, buffer->data, size2);
		}
		else
		{
			memcpy (data, buffer->data + buffer->fill_pos, size);
		}
	}

	// modifikujeme promenne
	buffer->fill_pos = (buffer->fill_pos + size) % buffer->buffer_size;
	buffer->free_space += size;
	buffer->messages--;
	if (buffer->free_space > buffer->buffer_size)
	{
		cms_ns_error ("libcms", "%s: buffer->buffer_size %u, buffer->free_pos %u, "
			"buffer->fill_pos %u, buffer->free_space %u", 
			buffer->name, buffer->buffer_size, buffer->free_pos, 
			buffer->fill_pos, buffer->free_space);
		_cms_round_buffer_clear (buffer);
		return -1;
	}

	return size;
}

int _cms_round_buffer_read_block (cms_round_buffer_t * buffer, 
	void * data, unsigned maxsize)
{
	int retval = -1;
	
	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// v bufferu nejsou zadna data cekame
	while (buffer->free_space == buffer->buffer_size)
	{
		if (buffer->flags & CMS_ROUND_BUFFER_READ_BLOCK)
		{
			g_cond_wait (buffer->cond, buffer->mutex);
		}
		else
		{
			goto end;
		}
	}

	// precteme data
	retval = __cms_round_buffer_read_block (buffer, data, maxsize);

	// zkontrolujeme vyuziti bufferu
	_cms_round_buffer_check_size (buffer);

end:
	// odemkneme pristup k bufferu
	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);

	return retval;
}

int __cms_round_buffer_write_block (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{
	unsigned size1, size2;

	// zkopirujeme do bufferu delku dat
	buffer->data[buffer->free_pos] = size & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->data[buffer->free_pos] = (size >> 8) & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->data[buffer->free_pos] = (size >> 16) & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->data[buffer->free_pos] = (size >> 24) & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->free_space -= 4;
	
	// zkopirujeme do bufferu data
	if (buffer->free_pos + size > buffer->buffer_size)
	{
		size2 = buffer->free_pos + size - buffer->buffer_size;
		size1 = size - size2;
		memcpy (buffer->data + buffer->free_pos, data, size1);
		memcpy (buffer->data, (char *) data + size1, size2);
	}
	else
	{
		memcpy (buffer->data + buffer->free_pos, data, size);
	}

	// modifikujeme promenne
	buffer->free_pos = (buffer->free_pos + size) % buffer->buffer_size;
	buffer->free_space -= size;
	buffer->messages++;

	return size;
}

int _cms_round_buffer_write_block (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{	
	int retval = -1;
	
	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// v bufferu neni dost mista
	while (size + 4 > buffer->free_space)
	{
		// zvetsime velikost bufferu
		if ((buffer->flags & CMS_ROUND_BUFFER_WRITE_GROW)
			&& buffer->buffer_size < buffer->max_buffer_size)
		{
			_cms_round_buffer_set_size (buffer, 
				buffer->max_buffer_size < buffer->buffer_size 
				* 2 ? buffer->max_buffer_size : buffer->buffer_size * 2);
		}
		// vyhodime data za zacatku bufferu
		else if ((buffer->flags & CMS_ROUND_BUFFER_WRITE_THROW)
			&& buffer->free_space < buffer->buffer_size)
		{
			cms_ns_warning ("libcms", "throwing data (%s)", buffer->name);
			__cms_round_buffer_read_block (buffer, NULL, INT_MAX);
		}
		// pockame na uvolneni bufferu
		else if (buffer->flags & CMS_ROUND_BUFFER_WRITE_BLOCK)
		{
			if (size + 4 > buffer->buffer_size)
			{
				cms_ns_error ("libcms", "small buffer (%s)", buffer->name);
				goto end;
			}
			g_cond_wait (buffer->cond, buffer->mutex);
		}
		// zahodime data
		else
		{
			cms_ns_error ("libcms", "not enough space in buffer (%d,%s,0x%x,%d,%d)", 
				size, buffer->name, buffer->flags, 
				buffer->buffer_size, buffer->max_buffer_size);
			goto end;
		}
	}

	// zapiseme do bufferu
	retval = __cms_round_buffer_write_block (buffer, data, size);

	// zapamatujeme si obsazeni bufferu
	buffer->max_fill_space = MAX (buffer->max_fill_space, 
		buffer->buffer_size - buffer->free_space);

end:
	// odemkneme pristup k bufferu a posleme signal
	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);

	return retval;
}

void _cms_round_buffer_time_check (cms_round_buffer_t * buffer)
{
	unsigned size;
	unsigned pos;
	time_t time;
	GTimeVal tv;

	g_get_current_time (&tv);
	
	while (buffer->free_space < buffer->buffer_size)
	{
		// precteme cas
		time = buffer->data[pos = buffer->fill_pos];
		pos = (pos + 1) % buffer->buffer_size;
		time |= ((unsigned) buffer->data[pos]) << 8;
		pos = (pos + 1) % buffer->buffer_size;
		time |= ((unsigned) buffer->data[pos]) << 16;
		pos = (pos + 1) % buffer->buffer_size;
		time |= ((unsigned) buffer->data[pos]) << 24;
		pos = (pos + 1) % buffer->buffer_size;

		// zprava je relativne nova, zustane
		if (time >= tv.tv_sec - buffer->max_buffer_time)
		{
			break;
		}

		// zjistime delku dat v bufferu
		size = buffer->data[pos];
		pos = (pos + 1) % buffer->buffer_size;
		size |= ((unsigned) buffer->data[pos]) << 8;
		pos = (pos + 1) % buffer->buffer_size;
		size |= ((unsigned) buffer->data[pos]) << 16;
		pos = (pos + 1) % buffer->buffer_size;
		size |= ((unsigned) buffer->data[pos]) << 24;
		pos = (pos + 1) % buffer->buffer_size;

		// preskocime zpravu
		buffer->fill_pos = (pos + size) % buffer->buffer_size;
		buffer->free_space += 8 + size;

		cms_ns_warning ("libcms", "old packet in time buffer");
	}
}

int __cms_round_buffer_read_time_block (cms_round_buffer_t * buffer, 
	void * data, unsigned maxsize)
{
	buffer->fill_pos = (buffer->fill_pos + 4) % buffer->buffer_size;
	buffer->free_space += 4;
	return __cms_round_buffer_read_block (buffer, data, maxsize);
}

int _cms_round_buffer_read_time_block (cms_round_buffer_t * buffer, 
	void * data, unsigned maxsize)
{
	int retval = -1;

	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// zahodime stare packety
	if (buffer->max_buffer_time && buffer->free_space < buffer->buffer_size)
	{
		_cms_round_buffer_time_check (buffer);
	}
	
	// v bufferu nejsou zadna data cekame
	while (buffer->free_space == buffer->buffer_size)
	{
		if (buffer->flags & CMS_ROUND_BUFFER_READ_BLOCK)
		{
			g_cond_wait (buffer->cond, buffer->mutex);
		}
		else
		{
			goto end;
		}
	}

	// precteme data
	retval = __cms_round_buffer_read_time_block (buffer, data, maxsize);

	// zkontrolujeme vyuziti bufferu
	_cms_round_buffer_check_size (buffer);

end:
	// odemkneme pristup k bufferu
	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);

	return retval;
}

int __cms_round_buffer_write_time_block (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{
	GTimeVal tv;

	g_get_current_time (&tv);
	
	// zkopirujeme do bufferu cas
	buffer->data[buffer->free_pos] = tv.tv_sec & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->data[buffer->free_pos] = (tv.tv_sec >> 8) & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->data[buffer->free_pos] = (tv.tv_sec >> 16) & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->data[buffer->free_pos] = (tv.tv_sec >> 24) & 0xff;
	buffer->free_pos = (buffer->free_pos + 1) % buffer->buffer_size;
	buffer->free_space -= 4;

	return __cms_round_buffer_write_block (buffer, data, size);
}

int _cms_round_buffer_write_time_block (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{
	int retval = -1;

	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// zahodime stare packety
	if (buffer->max_buffer_time && buffer->free_space < buffer->buffer_size)
	{
		_cms_round_buffer_time_check (buffer);
	}
	
	// v bufferu neni dost mista
	while (size + 8 > buffer->free_space)
	{
		// zvetsime velikost bufferu
		if ((buffer->flags & CMS_ROUND_BUFFER_WRITE_GROW)
			&& buffer->buffer_size < buffer->max_buffer_size)
		{
			_cms_round_buffer_set_size (buffer, 
				buffer->max_buffer_size < buffer->buffer_size 
				* 2 ? buffer->max_buffer_size : buffer->buffer_size * 2);
		}
		// vyhodime data za zacatku bufferu
		else if ((buffer->flags & CMS_ROUND_BUFFER_WRITE_THROW)
			&& buffer->free_space < buffer->buffer_size)
		{
			cms_ns_warning ("libcms", "throwing data (%s)", buffer->name);
			__cms_round_buffer_read_time_block (buffer, NULL, INT_MAX);
		}
		// pockame na uvolneni bufferu
		else if (buffer->flags & CMS_ROUND_BUFFER_WRITE_BLOCK)
		{
			if (size + 8 > buffer->buffer_size)
			{
				cms_ns_error ("libcms", "small buffer (%s)", buffer->name);
				goto end;
			}
			g_cond_wait (buffer->cond, buffer->mutex);
		}
		// zahodime data
		else
		{
			cms_ns_error ("libcms", "not enough space in buffer (%s,%d)", 
				buffer->name, buffer->buffer_size);
			goto end;
		}
	}

	// zapiseme do bufferu
	retval = __cms_round_buffer_write_time_block (buffer, data, size);

	// zapamatujeme si obsazeni bufferu
	buffer->max_fill_space = MAX (buffer->max_fill_space, 
		buffer->buffer_size - buffer->free_space);

end:
	// odemkneme pristup k bufferu a posleme signal
	g_mutex_unlock (buffer->mutex);
	g_cond_broadcast (buffer->cond);

	return retval;
}

int cms_round_buffer_read (cms_round_buffer_t * buffer, 
	void * data, unsigned size)
{
	switch (buffer->type)
	{
		case CMS_ROUND_BUFFER_TYPE_RAW:
		{
			return _cms_round_buffer_read_raw (buffer, data, size);
		}
		case CMS_ROUND_BUFFER_TYPE_BLOCK:
		{
			return _cms_round_buffer_read_block (buffer, data, size);
		}
		case CMS_ROUND_BUFFER_TYPE_TIME_BLOCK:
		{
			return _cms_round_buffer_read_time_block (buffer, data, size);
		}
	}

	return -1;
}

int cms_round_buffer_write (cms_round_buffer_t * buffer, 
	const void * data, unsigned size)
{
	switch (buffer->type)
	{
		case CMS_ROUND_BUFFER_TYPE_RAW:
		{
			return _cms_round_buffer_write_raw (buffer, data, size);
		}
		case CMS_ROUND_BUFFER_TYPE_BLOCK:
		{
			return _cms_round_buffer_write_block (buffer, data, size);
		}
		case CMS_ROUND_BUFFER_TYPE_TIME_BLOCK:
		{
			return _cms_round_buffer_write_time_block (buffer, data, size);
		}
	}

	return -1;
}

void cms_round_buffer_wait_for_empty (cms_round_buffer_t * buffer)
{
	// zamkneme pristup k bufferu
	g_mutex_lock (buffer->mutex);

	// cekame na prazdny buffer
	while (buffer->free_space < buffer->buffer_size)
	{
		g_cond_wait (buffer->cond, buffer->mutex);
	}
			
	// odemkneme pristup k bufferu
	g_mutex_unlock (buffer->mutex);
	
}

