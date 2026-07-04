#ifndef _C_CMS_CLIENT_H
#define _C_CMS_CLIENT_H

#include "cms_header.h"
#include "cms_buffer.h"

G_BEGIN_DECLS

struct cms_client_s
{
	unsigned magic;

	GMutex * mutex;

	gboolean server;
	gboolean init;
	gboolean connected;
	gboolean pollout;
	int socket;
	int socket2;

	unsigned connection_id;

	char * port;
	int send_recv_flags;
	enum CMS_TCP_MODE tcp_mode;
	unsigned ping_timeout;
	unsigned ping_interval;
	enum CMS_PING_TYPE ping_type;

	struct addrinfo * addrinfo;
	struct sockaddr * sockaddr;
	socklen_t sockaddrlen;
	gboolean loopback;
	
	char remote_addr[INET6_ADDRSTRLEN];
	char local_addr[INET6_ADDRSTRLEN];
	unsigned short remote_port;
	unsigned short local_port;

	cms_round_buffer_t * send_buffer;
	cms_round_buffer_t * recv_buffer;
	cms_round_buffer_t * external_recv_buffer;
	unsigned recv_time_count;

	// na tyto promenne mohou sahat 
	// pouze funkce volane z epollu
	unsigned char * send_data;
	unsigned send_data_size;
	unsigned char * recv_data;
	unsigned recv_data_size;
};

typedef struct cms_client_s cms_client_t;

// vytvori objekt pro tcp komunikaci na strane klienta
cms_client_t * cms_client_new (
	int send_recv_flags,
	const char * addr, 
	const char * port, 
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size,
	cms_round_buffer_t * recv_buffer);

// vytvori objekt pro tcp komunikaci na strane serveru
cms_client_t * cms_client_new_from_server (
	int send_recv_flags,
	int socket, 
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size,
	cms_round_buffer_t * recv_buffer,
	unsigned connection_id);

// smaze objekt
void cms_client_delete (cms_client_t * client);

// zahaji cinnost objektu
void cms_client_init (cms_client_t * client);

// ukonci cinnost objektu
void cms_client_shutdown (cms_client_t * client);

// vrati priznak, zda existuje spojeni
gboolean cms_client_is_connected (cms_client_t * client);

// pokusi se navazat tcp spojeni
gboolean cms_client_connect (cms_client_t * client);

// rozpoji tcp spojeni
void cms_client_disconnect (cms_client_t * client);

// cekame na odeslani vsech dat
void cms_client_flush (cms_client_t * client);

// vrati lokalni a vzdalenou adresu a port
const char * cms_client_get_local_addr (cms_client_t * client);
unsigned short cms_client_get_local_port (cms_client_t * client);
const char * cms_client_get_remote_addr (cms_client_t * client);
unsigned short cms_client_get_remote_port (cms_client_t * client);

// odesle data
int cms_client_send (cms_client_t * client, const void * data, unsigned size);
	
// prijme data
int cms_client_recv (cms_client_t * client, void * data, unsigned maxsize);

G_END_DECLS

#endif /* _C_CMS_CLIENT_H */
