#ifndef _C_COM_H
#define _C_COM_H

#include "cms_header.h"

#ifdef G_OS_UNIX

G_BEGIN_DECLS

enum CSM_COM_TYPE
{
	CMS_COM_TYPE_DEFAULT,
	CMS_COM_TYPE_START_STOP,
	CMS_COM_TYPE_AMS,
	CMS_COM_TYPE_KOMAR,
	CMS_COM_TYPE_VITEK,
};

struct cms_com_s
{
	unsigned magic;
	enum CSM_COM_TYPE type;
	char * device;
	unsigned int speed;
	unsigned short data_bits;
	unsigned short stop_bits;
	unsigned char parity;
	unsigned short break_time;
	int handler;
	struct termios new_termios;
	struct termios old_termios;
	char * msg_start;
	char * msg_stop;
};

typedef struct cms_com_s cms_com_t;

cms_com_t * cms_com_new (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time);
cms_com_t * cms_com_new_start_stop (const char * device, 
	unsigned int speed, unsigned short data_bits, unsigned short stop_bits, 
	int parity, unsigned short break_time, const char * msg_start,
	const char * msg_stop);
cms_com_t * cms_com_new_ams (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time);
cms_com_t * cms_com_new_komar (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time);
cms_com_t * cms_com_new_vitek (const char * device, unsigned int speed, 
	unsigned short data_bits, unsigned short stop_bits, 
	unsigned char parity, unsigned short break_time);
void cms_com_delete (cms_com_t * com);
void cms_com_close (cms_com_t * com);

void cms_com_set_modem (cms_com_t * com, int modem); // bit0-RTS, bit1-DTR
int cms_com_get_modem (cms_com_t * com); // bit0-CTS, bit1-DSR, bit2-RI, bit3-ERROR

int cms_com_read (cms_com_t * com, void * data, unsigned size);
int cms_com_write (cms_com_t * com, const void * data, unsigned maxsize);

G_END_DECLS

#endif /* G_OS_UNIX */

#endif /* _C_COM_H */
