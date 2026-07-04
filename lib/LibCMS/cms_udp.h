#ifndef _CMS_UDP_H
#define _CMS_UDP_H

#include "cms_header.h"
#include "cms_buffer.h"

G_BEGIN_DECLS

struct cms_udp_s
{
	unsigned magic;

	gboolean init;	
	int socket;
	gboolean pollout;

	char * recv_port;
	char * send_port;
	int send_recv_flags;
	
	struct addrinfo * recv_addrinfo;
	struct addrinfo * send_addrinfo;
	GList * source_addrinfo;

	gboolean bonding;
	time_t bonding_recv_time;
	unsigned short bonding_send_count;
	unsigned short bonding_recv_count;
	
	cms_round_buffer_t * send_buffer;
	cms_round_buffer_t * recv_buffer;
	cms_round_buffer_t * external_recv_buffer;
};

typedef struct cms_udp_s cms_udp_t;

// vytvori objekt pro udp unicast nebo broadcast
cms_udp_t * cms_udp_new (
	int send_recv_flags,
	const char * recv_addr, 
	const char * recv_port, 
	const char * send_addr, 
	const char * send_port,
	const char ** source_addr,
	cms_round_buffer_t * recv_buffer);

// vytvorim objekt pro udp mutlicast
cms_udp_t * cms_udp_new_multicast (
	int send_recv_flags,
	const char * recv_multicast_addr, 
	const char * recv_local_addr,
	const char * recv_port, 
	const char * send_multicast_addr,
	const char * send_local_addr, 
	const char * send_port,
	cms_round_buffer_t * recv_buffer);

// smaze objekt
void cms_udp_delete (cms_udp_t * udp);

// zahaji cinnost objektu
void cms_udp_init (cms_udp_t * udp);

// ukonci cinnost objektu
void cms_udp_shutdown (cms_udp_t * udp);

// zmeni hodnotu ttl
void cms_udp_set_time_to_live (cms_udp_t * udp, unsigned char ttl);

// zapne pouziti citace na konci datovych packetu
void cms_udp_set_use_bonding (cms_udp_t * udp);

// cekame na odeslani vsech dat
void cms_udp_flush (cms_udp_t * udp);

// odesle data
int cms_udp_send (cms_udp_t * udp, const void * data, unsigned size);

// prijme data
int cms_udp_recv (cms_udp_t * udp, void * data, unsigned maxsize);

G_END_DECLS

#endif /* _CMS_UDP_H */
