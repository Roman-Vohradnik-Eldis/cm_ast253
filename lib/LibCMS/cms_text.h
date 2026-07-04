#ifndef _CMS_TEXT_H
#define _CMS_TEXT_H

#include "cms_header.h"

G_BEGIN_DECLS

struct cms_text_s
{
	unsigned magic;
	char * path;
	gboolean write;
	unsigned maxsize;
	unsigned timeout;
	FILE * file;
};

typedef struct cms_text_s cms_text_t;

cms_text_t * cms_text_new_read (const char * path, unsigned maxsize, unsigned timeout);
cms_text_t * cms_text_new_write (const char * path);
void cms_text_delete (cms_text_t * file);
void cms_text_close (cms_text_t * file);

int cms_text_read (cms_text_t * file, void * data, unsigned maxsize);
int cms_text_write (cms_text_t * file, const void * data, unsigned size);

G_END_DECLS

#endif /* _CMS_TEXT_H */
