#include "cms_com.h"
#include "cms_debug.h"
#include "cms_misc.h"

#ifdef G_OS_UNIX

#define CMS_COM_MAGIC 0x72060903

cms_com_t * cms_com_new (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time)
{
	cms_com_t * com;
	unsigned long com_speed;
	unsigned long com_data_bits;
	unsigned long com_stop_bits;
	unsigned long com_parity;
	unsigned int ioret;
	unsigned int status;
	int i;

	// budeme ignorovat signal SIGIO	
	sigset_t sigset;
	sigaddset (&sigset, 29);
	sigprocmask (SIG_BLOCK, &sigset, NULL);

	com = (cms_com_t *) g_malloc0 (sizeof (cms_com_t));
	if (!com)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}
	
	com->device = g_strdup (device);
	if (!com->device)
	{
		cms_ns_error ("libcms", "g_strdup error");
		goto failed;
	}
	
	com->handler = -1;
	com->type = CMS_COM_TYPE_DEFAULT;	
	com->speed = speed;
	com->data_bits = data_bits;
	com->stop_bits = stop_bits;
	com->parity = parity;
	com->break_time = break_time;

	for (i = 0; i < 5; i++)
	{
		com->handler = open (device, O_RDWR | O_NOCTTY);// | O_NONBLOCK);
		if (com->handler == -1)
		{
			cms_ns_error ("libcms", "unable to open device (%s,%d,%s)", 
				device, errno, strerror (errno));
			goto failed;
		}

		fcntl (com->handler, F_SETFL, FASYNC);
		tcgetattr (com->handler, &com->old_termios);
		status = ioctl (com->handler, TIOCMGET, &ioret);
		if (status == 0)
		{
			break;
		}
	}

	if (status != 0)
	{
		cms_ns_error ("libcms", "ioctl TIOCMGET error (%s,%d,%s)", 
			device, errno, strerror (errno));
	}

	switch (speed)
	{
		case 50: com_speed = B50; break;
		case 75: com_speed = B75; break;
		case 110: com_speed = B110; break;
		case 134: com_speed = B134; break;
		case 150: com_speed = B150; break;
		case 200: com_speed = B200; break;
		case 300: com_speed = B300; break;
		case 600: com_speed = B600; break;
		case 1200: com_speed = B1200; break;
		case 2400: com_speed = B2400; break;
		case 4800: com_speed = B4800; break;
		case 9600: com_speed = B9600; break;
		case 19200: com_speed = B19200; break;
		case 38400: com_speed = B38400; break;
		case 57600: com_speed = B57600; break;
		case 115200: com_speed = B115200; break;
		default: com_speed = B0;
	}
	
	if (com_speed == B0)
	{
		cms_ns_error ("libcms", "wrong com speed (%d)", speed);
		goto failed;
	}

	switch (data_bits)
	{
		case 5: com_data_bits = CS5; break;
		case 6: com_data_bits = CS6; break;
		case 7: com_data_bits = CS7; break;
		case 8: com_data_bits = CS8; break;
		default: com_data_bits = CS8;
	}

	if (stop_bits == 2)
	{
		com_stop_bits = CSTOPB;
	}
	else
	{
		com_stop_bits = B0;							//to je 0x00000000
	}
	
	if (parity == 2)
	{
		com_parity = PARENB;
	}
	if (parity == 1)
	{
		com_parity = PARENB | PARODD;
	}
	else
	{
		com_parity = B0;								//0x00000000
	}

	com->new_termios.c_cflag = com_speed | com_data_bits | com_stop_bits | com_parity | CLOCAL | CREAD;
	com->new_termios.c_iflag = 0;						//INPCK | PARMRK ;
	com->new_termios.c_oflag = 0;
	com->new_termios.c_lflag = CSIZE | com_data_bits;
	com->new_termios.c_cc[VMIN] = 0;
	com->new_termios.c_cc[VTIME] = break_time;
	tcflush (com->handler, TCIFLUSH);
	tcsetattr (com->handler, TCSANOW, &com->new_termios);

	com->magic = CMS_COM_MAGIC;
	return com;

failed:
	if (com)
	{
		if (com->device)
		{
			g_free (com->device);
		}
		if (com->handler != -1)
		{
			close (com->handler);
		}
		g_free (com);
	}
	return NULL;
}

cms_com_t * cms_com_new_start_stop (const char * device, 
	unsigned int speed, unsigned short data_bits, unsigned short stop_bits, 
	int parity, unsigned short break_time, const char * msg_start,
	const char * msg_stop)
{
	cms_com_t * com = cms_com_new (device, speed, data_bits, stop_bits, parity, break_time);
	if (!com)
	{
		return NULL;
	}
	
	com->type = CMS_COM_TYPE_START_STOP;
	com->msg_start = g_strdup (msg_start);
	com->msg_stop = g_strdup (msg_stop);
	return com;
}

cms_com_t * cms_com_new_ams (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time)
{
	cms_com_t * com = cms_com_new (device, speed, data_bits, stop_bits, parity, break_time);
	if (!com)
	{
		return NULL;
	}
	
	com->type = CMS_COM_TYPE_AMS;
	return com;
}

cms_com_t * cms_com_new_komar (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time)
{
	cms_com_t * com = cms_com_new (device, speed, data_bits, stop_bits, parity, break_time);
	if (!com)
	{
		return NULL;
	}
	
	com->type = CMS_COM_TYPE_KOMAR;
	return com;
}

cms_com_t * cms_com_new_vitek (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time)
{
	cms_com_t * com = cms_com_new (device, speed, data_bits, stop_bits, parity, break_time);
	if (!com)
	{
		return NULL;
	}
	
	com->type = CMS_COM_TYPE_VITEK;
	return com;
}

void cms_com_delete (cms_com_t * com)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return;
	}

	com->magic = 0;

	if (com->handler != -1)
	{
		tcsetattr (com->handler, TCSANOW, &com->old_termios);
		close (com->handler);
	}
	if (com->device)
	{
		g_free (com->device);
	}
	if (com->msg_start)
	{
		g_free (com->msg_start);
	}
	if (com->msg_stop)
	{
		g_free (com->msg_stop);
	}
	g_free (com);
}

void cms_com_close (cms_com_t * com)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return;
	}

	if (com->handler != -1)
	{
		tcsetattr (com->handler, TCSANOW, &com->old_termios);
		close (com->handler);
		com->handler = -1;	
	}
}

int cms_com_get_modem (cms_com_t * com)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned int ioret;
	unsigned int status;
	int retval = 0;

	status = ioctl (com->handler, TIOCMGET, &ioret);
	if (status == 0)
	{
		if (ioret & TIOCM_CTS)
		{
			retval |= 1;
		}
		if (ioret & TIOCM_DSR)
		{
			retval |= 2;
		}
		if (ioret & TIOCM_RNG)
		{
			retval |= 4;
		}
	}
	else
	{
		cms_ns_error ("libcms", "ioctl TIOCMGET error (%s,%d,%s)", 
			com->device, errno, strerror (errno));
		retval = 8;
	}
	
	return retval;
}

void cms_com_set_modem (cms_com_t * com, int mdm)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return;
	}

	unsigned int ioret;
	unsigned int status;

	status = ioctl (com->handler, TIOCMGET, &ioret);
	if (status == 0)
	{
		ioret &= ~TIOCM_RTS;
		ioret &= ~TIOCM_DTR;
		if (mdm & 1)
		{
			ioret |= TIOCM_RTS;
		}
		if (mdm & 2)
		{
			ioret |= TIOCM_DTR;
		}
		ioctl (com->handler, TIOCMSET, &ioret);
	}
	else
	{
		cms_ns_error ("libcms", "ioctl TIOCMGET error (%s,%d,%s)", 
			com->device, errno, strerror (errno));
	}
}

int cms_com_write_default (cms_com_t * com, const void * data, unsigned size)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	int status;
	size_t size2;

	size2 = 0;
	while (size2 < size)
	{
		status = write (com->handler, data, size);
		if (status == -1)
		{
			return -1;
		}
		size2 += status;
	}

	tcdrain (com->handler);
	return size2;
}

int cms_com_read_default (cms_com_t * com, void *data, unsigned maxsize)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	return read (com->handler, data, maxsize);
}

int cms_com_read_start_stop (cms_com_t * com, 
	void * data, unsigned maxsize)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned i = 0;
	unsigned j = 0;
	int status;

	while (com->msg_start[i] && i < maxsize)
	{
		// cteme z comu
		status = cms_com_read_default (com, ((char *)data) + i, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// precetlo, ale neni to zacatek zpravy
		if (((char *)data)[i] != com->msg_start[i])
		{
			i = 0;
			continue;
		}

		// precetlo, a je to soucast hlavicky zpravy, jdeme dal
		i++;
	}

	while (com->msg_stop[j] && i < maxsize)
	{
		// cteme z comu
		status = cms_com_read_default (com, ((char *)data) + i, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// precetlo se, ale neni to konec zpravy
		if (((char *)data)[i] != com->msg_stop[j])
		{
			i++;
			j = 0;
			continue;
		}

		// precetlo se, a je to konec zpravy
		i++;
		j++;
	}

	return i;
}

int cms_com_write_start_stop (cms_com_t * com, 
	const void * data, unsigned size)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned i, j;
	
	for (i = 0; com->msg_start[i]; i++)
	{
		if (((char *)data)[i] != com->msg_start[i])
		{
			cms_ns_warning ("libcms", "start of message is missing");
			//return 0;
		}
	}

	for (i = 0, j = size - strlen (com->msg_stop) - 1; com->msg_stop[i]; i++, j++)
	{
		if (((char *)data)[j] != com->msg_stop[i])
		{
			cms_ns_warning ("libcms", "end of message is missing");
			//return 0;
		}
	}

	return cms_com_write_default (com, data, size);
}

int cms_com_read_ams (cms_com_t * com, void * data, unsigned maxsize)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned char * buffer = (unsigned char *) data;
	unsigned i = 1;
	unsigned j = 0;
	unsigned sum = 0;
	unsigned checksum = 0;
	int status;

	// hledame zacatek zpravy
	while (TRUE)
	{
		// cteme z comu
		status = cms_com_read_default (com, buffer, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// zacatek zpravy
		if (buffer[0] == 0x01)
		{
			break;
		}
	}

	// precteme hlavicku
	while (TRUE)
	{
		// zkontrolujeme delku
		if (i == maxsize)
		{
			cms_ns_warning ("libcms", "message is too long");
			return -1;
		}
		
		// cteme z comu
		status = cms_com_read_default (com, buffer + i, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}
		
		// konec hlavicky, zacatek textu
		if (buffer[i++] == 0x02)
		{
			break;
		}
	}

	// precteme zpravu
	while (TRUE)
	{
		// zkontrolujeme delku
		if (i == maxsize)
		{
			cms_ns_warning ("libcms", "message is too long");
			return -1;
		}

		// cteme z comu
		status = cms_com_read_default (com, buffer + i, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// konec textu, zacatek checksumu
		if (buffer[i] == 0x03)
		{
		    i++;
		    break;
		}
		
		// pricteme checksum
		sum += buffer[i++];
	}
	
	// precteme checksum
	while (TRUE)
	{
		// zkontrolujeme delku
		if (i == maxsize)
		{
			cms_ns_warning ("libcms", "message is too long");
			return -1;
		}
		
		// cteme z comu
		status = cms_com_read_default (com, buffer + i, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// konec checksumu, konec zpravy
		if (buffer[i] == 0x04)
		{
		    i++;
		    break;
		}
		
		// pricteme checksum
		checksum = checksum * 16;
		if (g_ascii_isdigit (buffer[i]))
		{
		    checksum += g_ascii_digit_value (buffer[i]);
		}
		else if (g_ascii_isxdigit (buffer[i]))
		{
		    checksum += g_ascii_toupper (buffer[i]) - 'A' + 10;
		}
		else
		{
		    cms_ns_warning ("libcms", "wrong character %c", buffer[i]);
		}
		
		i++;
		j++;
	}

	// zkontrolujeme checksum
	if (j == 1)
	{
		sum &= 0xff;
	}
	else if (j == 2)
	{
		sum &= 0xffff;
	}
	else if (j == 4)
	{
		sum &= 0xffffffff;
	}
	else
	{
		cms_ns_warning ("libcms", "wrong checksum size: %d", j);
		return -1;
	}
	
	if (checksum != sum)
	{
		cms_ns_warning ("libcms", "checksum error %i: %x != %x", i, checksum, sum);
		return -1;
	}

	return i;
}

int cms_com_write_ams (cms_com_t * com, const void * data, unsigned size)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	return cms_com_write_default (com, data, size);
}

int cms_com_read_komar (cms_com_t * com, void * data, unsigned maxsize)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned char buffer[2];
	unsigned size;
	int status;
	int i = 0;

	// cteme hlavicku
	i = 0;
	while (i < 3)
	{
		// cteme z comu
		status = cms_com_read_default (com, buffer, 1);

		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// precetlo se, ale neni to zacatek zpravy
		if (buffer[0] != 0x55)
		{
			i = 0;
			continue;
		}

		// prectlo se, je to zacatek zpravy
		i++;
	}

	// cteme delku zpravy
	i = 0;
	while (i < 2)
	{
		status = cms_com_read_default (com, buffer + i, 2 - i);
		if (status == -1)
		{
			return -1;
		}
		i += status;
	}

	size = buffer[0] | (((unsigned)buffer[1]) << 8);

	if (size > maxsize)
	{
		cms_ns_warning ("libcms", "message is too long");
		return -1;
	}
	
	// cteme zpravu
	i = 0;
	while (i < size)
	{
		status = cms_com_read_default (com, ((unsigned char *)data) + i, size - i);
		if (status == -1)
		{
			return -1;
		}
		i += status;
	}

	// cteme checksum
	i = 0;
	while (i < 1)
	{
		status = cms_com_read_default (com, buffer + i, 1 - i);
		if (status == -1)
		{
			return -1;
		}
		i += status;
	}

	// spocitame checksum
	buffer[1] = 0;
	for (i = 0; i < size; i++)
	{
		buffer[1] += ((unsigned char *)data)[i];
	}

	// zkontrolujeme checksum
	if (((buffer[0] + buffer[1]) & 0xff) != 0)
	{
		cms_ns_warning ("libcms", "checksum doesn't match %d %d", 
			buffer[0], buffer[1], (buffer[0] + buffer[1]) & 0xff);
		return -1;
	}

	return size;
}

int cms_com_write_komar (cms_com_t * com, const void * data, unsigned size)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned char buffer[size + 5];
	unsigned char checksum = 0;
	int status;
	int i;
	
	// zapiseme hlavicku
	buffer[0] = buffer[1] = buffer[2] = 0x55;

	// zapiseme delku zpravy
	buffer[3] = size & 0xff;
	buffer[4] = (size >> 8) & 0xff;

	// zapiseme zpravu
	memcpy (buffer + 5, data, size);

	// zapiseme checksum
	for (i = 0; i < size; i++) 
	{
		checksum += ((unsigned char *)data)[i];
	}
	buffer[size + 5] = ~checksum + 1;

	// odesleme zpravu
	status = cms_com_write_default (com, buffer, (unsigned int) size + 6);
	if (status == -1)
	{
		return -1;
	}

	return size;
}

int cms_com_read_vitek (cms_com_t * com, void * data, unsigned maxsize)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	unsigned char buffer[260];
	unsigned size;
	int status;
	int i = 0;

	// cteme hlavicku
	i = 0;
	while (i < 3)
	{
		// cteme z comu
		status = cms_com_read_default (com, buffer + i, 1);
		cms_debug ("read %d", status);
		
		// nastala chyba pri cteni
		if (status == -1)
		{
			return -1;
		}

		// nic se neprecetlo
		if (status == 0)
		{
			continue;
		}

		// precetlo se, ale neni to zacatek zpravy
		if (buffer[i] != 0x55)
		{
			i = 0;
			continue;
		}

		// prectlo se, je to zacatek zpravy
		i++;
	}

	// cteme delku zpravy
	i = 0;
	while (i < 1)
	{
		status = cms_com_read_default (com, buffer + 3 + i, 1);
		cms_debug ("read %d", status);
		if (status == -1)
		{
			return -1;
		}
		i += status;
	}

	size = buffer[3];

	if (size + 5 > maxsize)
	{
		cms_ns_warning ("libcms", "message is too long");
		return -1;
	}
	
	// cteme zpravu
	i = 0;
	size--;
	while (i < size)
	{
		status = cms_com_read_default (com, buffer + 4 + i, size - i);
		cms_debug ("read %d", status);
		if (status == -1)
		{
			return -1;
		}
		i += status;
	}

	// cteme checksum
	i = 0;
	while (i < 1)
	{
		status = cms_com_read_default (com, buffer + 4 + size + i, 1);
		cms_debug ("read %d", status);
		if (status == -1)
		{
			return -1;
		}
		i += status;
	}

	memcpy (data, buffer, size + 5);
	return size + 5;
}

int cms_com_write_vitek (cms_com_t * com, const void * data, unsigned size)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	return cms_com_write_default (com, data, size);
}

int cms_com_read (cms_com_t * com, void * data, unsigned size)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	switch (com->type)
	{
		case CMS_COM_TYPE_DEFAULT:
		{
			return cms_com_read_default (com, data, size);
		}

		case CMS_COM_TYPE_START_STOP:
		{
			return cms_com_read_start_stop (com, data, size);
		}

		case CMS_COM_TYPE_AMS:
		{
			return cms_com_read_ams (com, data, size);
		}

		case CMS_COM_TYPE_KOMAR:
		{
			return cms_com_read_komar (com, data, size);
		}

		case CMS_COM_TYPE_VITEK:
		{
			return cms_com_read_vitek (com, data, size);
		}
	}

	return -1;
}

int cms_com_write (cms_com_t * com, const void * data, unsigned maxsize)
{
	if (!com || com->magic != CMS_COM_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", com->magic);
		return 0;
	}

	switch (com->type)
	{
		case CMS_COM_TYPE_DEFAULT:
		{
			return cms_com_write_default (com, data, maxsize);
		}

		case CMS_COM_TYPE_START_STOP:
		{
			return cms_com_write_start_stop (com, data, maxsize);
		}

		case CMS_COM_TYPE_AMS:
		{
			return cms_com_write_ams (com, data, maxsize);
		}

		case CMS_COM_TYPE_KOMAR:
		{
			return cms_com_write_komar (com, data, maxsize);
		}

		case CMS_COM_TYPE_VITEK:
		{
			return cms_com_write_vitek (com, data, maxsize);
		}
	}

	return -1;
}

#endif /* G_OS_UNIX */
