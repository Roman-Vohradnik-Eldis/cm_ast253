#ifndef _CMS_FILE_H
#define _CMS_FILE_H

#include "cms_header.h"

G_BEGIN_DECLS

struct cms_file_s
{
	unsigned magic;
	char * path;
	gboolean write;
	unsigned maxsize;
	unsigned timeout;
	int fd;
};

typedef struct cms_file_s cms_file_t;

cms_file_t * cms_file_new_read (const char * path, unsigned maxsize, unsigned timeout);
cms_file_t * cms_file_new_write (const char * path);
void cms_file_delete (cms_file_t * file);
void cms_file_close (cms_file_t * file);

int cms_file_read (cms_file_t * file, void * data, unsigned maxsize);
int cms_file_write (cms_file_t * file, const void * data, unsigned size);

G_END_DECLS

#endif /* _CMS_FILE_H */
