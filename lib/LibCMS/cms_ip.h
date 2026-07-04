#ifndef _CMS_IP_H
#define _CMS_IP_H

#include "cms_header.h"
#include "cms_buffer.h"
#include "cms_client.h"
#include "cms_com.h"
#include "cms_file.h"
#include "cms_server_2.h"
#include "cms_text.h"
#include "cms_udp.h"

G_BEGIN_DECLS

struct cms_ip_s
{
	unsigned magic;

	GMutex * mutex;

	gboolean run;
	gboolean init;

	int send_recv_flags;
	char * descriptors;
	unsigned max_buffer_size;
	unsigned max_buffer_time;

	GPtrArray * client;
	GPtrArray * client_thread;
	GPtrArray * server;
	GPtrArray * udp;

	cms_file_t * file;
	cms_text_t * text;
#ifdef G_OS_UNIX
	cms_com_t * com;
#endif

	cms_round_buffer_t * recv_buffer;

	char * status_name;
	const char * status_port_1;
	const char * status_port_2;
	unsigned status_time;
	unsigned * send_bytes;
	unsigned * send_packets;
	unsigned * recv_bytes;
	unsigned * recv_packets;
};

typedef struct cms_ip_s cms_ip_t;

// vytvori objekt pro tcp server
cms_ip_t * cms_ip_new (int send_recv_flags, const char * descriptors);

// smaze objekt
void cms_ip_delete (cms_ip_t * ip);

// zahaji cinnost objektu
void cms_ip_init (cms_ip_t * ip);

// ukonci cinnost objektu
void cms_ip_shutdown (cms_ip_t * ip);

// vrati priznak, zda existuje spojeni
gboolean cms_ip_is_connected (cms_ip_t * ip);

// vrati vzdalenou adresu
gchar * cms_ip_get_remote_addr (cms_ip_t * ip);

// vrati velikost dat v prijimacim bufferu
unsigned cms_ip_get_recv_buffer_fill_space (cms_ip_t * ip);

// vrati pocet zprav v prijimacim bufferu
unsigned cms_ip_get_recv_buffer_messages (cms_ip_t * ip);

// odesle data
int cms_ip_send (cms_ip_t * ip, const void * data, unsigned size);

// prijme data
int cms_ip_recv (cms_ip_t * ip, void * data, unsigned maxsize);

G_END_DECLS

#endif /* _CMS_IP_H */
