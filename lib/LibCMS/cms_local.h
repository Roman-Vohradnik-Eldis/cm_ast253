#ifndef _CMS_LOCAL_H
#define _CMS_LOCAL_H

#include "cms_header.h"
#include "cms_buffer.h"

G_BEGIN_DECLS

struct cms_local_s
{
	unsigned magic;
	struct sockaddr_un recv_addr;
	struct sockaddr_un send_addr;
	int socket;
};

typedef struct cms_local_s cms_local_t;

// vytvori objekt
cms_local_t * cms_local_new (const char * recvpath, const char * sendpath);

// smaze objekt
void cms_local_delete (cms_local_t * local);

// odesle data
int cms_local_send (cms_local_t * local, const void * data, unsigned size);

// prijme data
int cms_local_recv (cms_local_t * local, void * data, unsigned maxsize);

G_END_DECLS

#endif /* _CMS_LOCAL_H */
