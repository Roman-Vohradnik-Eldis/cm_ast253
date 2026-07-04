#ifndef _CMS_BUFFER_H
#define _CMS_BUFFER_H

#include "cms_header.h"

G_BEGIN_DECLS

enum CMS_ROUND_BUFFER_TYPE
{
	CMS_ROUND_BUFFER_TYPE_RAW,
	CMS_ROUND_BUFFER_TYPE_BLOCK,
	CMS_ROUND_BUFFER_TYPE_TIME_BLOCK,
};

// standartni chovani kruhoveho bufferu:
// 1) nejsou-li v bufferu zadna data, ctecimu procesu je vracena -1
// 2) neni-li v bufferu dostatek volneho mista, data jsou zahozena
// a zapisujicimu procesu je vracena -1

// nejsou-li v bufferu zadna data, cteci proces se zablokuje
#define CMS_ROUND_BUFFER_READ_BLOCK		0x00000001

// neni-li v bufferu pozadovane mnozstvi dat, cteci proces se zablokuje
#define CMS_ROUND_BUFFER_READ_BLOCK_ALL	0x00000002

// neni-li v bufferu dostatek volneho mista, zapisujici proces se zablokuje
#define CMS_ROUND_BUFFER_WRITE_BLOCK		0x00000004

// neni-li v bufferu dostatek volneho mista, buffer se zvetsi
#define CMS_ROUND_BUFFER_WRITE_GROW		0x00000008

// neni-li v bufferu dostatek volneho mista, buffer zahazuje nejstarsi data
#define CMS_ROUND_BUFFER_WRITE_THROW		0x00000010

struct cms_round_buffer_s
{
	unsigned magic;
	enum CMS_ROUND_BUFFER_TYPE type;
	char * name;
	unsigned max_buffer_size;
	unsigned max_buffer_time;
	unsigned flags;
	unsigned char * data;
	unsigned buffer_size;
	unsigned free_space;
	unsigned free_pos;
	unsigned fill_pos;
	unsigned messages;
	GMutex * mutex;
	GCond * cond;
	GTimeVal check_time;
	unsigned max_fill_space;
};

typedef struct cms_round_buffer_s cms_round_buffer_t;

cms_round_buffer_t * cms_round_buffer_new (const char * name, 
	unsigned maxsize, int flags);
cms_round_buffer_t * cms_round_buffer_new_with_block (const char * name, 
	unsigned maxsize, int flags);
cms_round_buffer_t * cms_round_buffer_new_with_time_block (const char * name, 
	unsigned maxsize, unsigned maxtime, int flags);

void cms_round_buffer_delete (cms_round_buffer_t * buffer);

gboolean cms_round_buffer_is_empty (cms_round_buffer_t * buffer);
unsigned cms_round_button_get_messages (cms_round_buffer_t * buffer);
void cms_round_buffer_set_flags (cms_round_buffer_t * buffer, int flags);
void cms_round_buffer_clear (cms_round_buffer_t * buffer);
unsigned cms_round_buffer_get_fill_space (cms_round_buffer_t * buffer);
unsigned cms_round_buffer_get_messages (cms_round_buffer_t * buffer);

int cms_round_buffer_read (cms_round_buffer_t * buffer, 
	void * data, unsigned size);
int cms_round_buffer_write (cms_round_buffer_t * buffer, 
	const void * data, unsigned size);

G_END_DECLS

#endif /* _CMS_BUFFER_H */
