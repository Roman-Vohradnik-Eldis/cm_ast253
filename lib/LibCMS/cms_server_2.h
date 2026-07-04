#ifndef _C_CMS_SERVER_2_H
#define _C_CMS_SERVER_2_H

#include "cms_header.h"
#include "cms_buffer.h"
#include "cms_server.h"

G_BEGIN_DECLS

struct cms_server_2_s
{
	unsigned magic;

	GMutex * mutex;

	gboolean run;
	gboolean init;
	int socket;

	unsigned last_connection_id;
	
	char * port;
	int send_recv_flags;
	unsigned ping;
	enum CMS_TCP_MODE tcp_mode;
	unsigned max_buffer_size;

	struct addrinfo * addrinfo;

	cms_round_buffer_t * recv_buffer;
	cms_round_buffer_t * external_recv_buffer;

	GThread * server_new_thread;
	GThread * server_delete_thread;
	GList * client_list;

	gboolean (* on_cms_client_new) (cms_client_t *, void *);
	void *on_client_new_data;
	gboolean (* on_cms_client_delete) (cms_client_t *, void *);
	void *on_client_delete_data;
};

typedef struct cms_server_2_s cms_server_2_t;

// vytvori objekt pro tcp server
cms_server_2_t * cms_server_2_new (
	int send_recv_flags, 
	const char * addr, 
	const char * port, 
	unsigned ping, 
	enum CMS_TCP_MODE mode,
	unsigned max_buffer_size,
	gboolean (* on_cms_client_new) (cms_client_t *, void *),
	void *on_client_new_data,
	gboolean (* on_cms_client_delete) (cms_client_t *, void *),
	void *on_client_delete_data,
	cms_round_buffer_t * recv_buffer);

// smaze objekt
void cms_server_2_delete (cms_server_2_t * server);

// zahaji cinnost objektu
void cms_server_2_init (cms_server_2_t * server);

// ukonci cinnost objektu
void cms_server_2_shutdown (cms_server_2_t * server);

// ukoncime konkretni spojeni
void cms_server_2_disconnect (cms_server_2_t * server, unsigned connection_id);

// vrati priznak, zda existuje spojeni
gboolean cms_server_2_is_connected (cms_server_2_t * server);

// vrati vzdalenou adresu
gchar * cms_server_2_get_remote_addr (cms_server_2_t * server, unsigned connection_id);

// vrati velikost dat v prijimacim bufferu
unsigned cms_server_2_get_recv_buffer_fill_space (cms_server_2_t * server);

// vrati pocet zprav v prijimacim bufferu
unsigned cms_server_2_get_recv_buffer_messages (cms_server_2_t * server);

// odesle data
int cms_server_2_send (cms_server_2_t * server, const void * buffer, 
	unsigned size, unsigned connection_id);

// prijme data
int cms_server_2_recv (cms_server_2_t * server, void * buffer, 
	unsigned maxsize, unsigned * connection_id);

G_END_DECLS

#endif /* _C_CMS_SERVER_2_H */
