#ifndef _C_CMS_SERVER_H
#define _C_CMS_SERVER_H

#include "cms_header.h"
#include "cms_client.h"

G_BEGIN_DECLS

struct cms_server_s
{
	unsigned magic;
	int send_recv_flags;
	char * address;
	char * port;
	int socket;
	struct addrinfo * addrinfo;
	unsigned ping;
	enum CMS_TCP_MODE tcp_mode;
	unsigned max_buffer_size;
	gboolean run;
};

typedef struct cms_server_s cms_server_t;

cms_server_t * cms_server_new (
	int send_recv_flags, 
	const char * addr, 
	const char * port,
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size);

void cms_server_delete (cms_server_t * server);
void cms_server_shutdown (cms_server_t * server);
cms_client_t * cms_server_wait_for_client (cms_server_t * server);

G_END_DECLS

#endif /* _C_CMS_SERVER_H */
