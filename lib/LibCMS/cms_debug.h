#ifndef _CMS_DEBUG_H
#define _CMS_DEBUG_H

#include "cms_header.h"

G_BEGIN_DECLS

#define CMS_DEBUG_LEVEL_ERROR	1
#define CMS_DEBUG_LEVEL_WARNING	2
#define CMS_DEBUG_LEVEL_INFO	3
#define CMS_DEBUG_LEVEL_DEBUG	4

struct cms_debug_settings_s
{
	unsigned level;
	gboolean data;
	gboolean color;
	gboolean timestamp;
};

typedef struct cms_debug_settings_s cms_debug_settings_t;

gboolean cms_debug_init (const char * init);
void cms_debug_server_init (unsigned port);
void cms_debug_set_project (const char * project);
void cms_debug_set_program (const char * program);
void cms_debug_done ();

void cms_debug_set_stdout (gboolean stdout);
void cms_debug_set_syslog (gboolean syslog);
gboolean cms_debug_get_stdout ();
gboolean cms_debug_get_syslog ();

void cms_debug_ns_set_level (const gchar * ns, unsigned level);
void cms_debug_ns_set_data (const gchar * ns, gboolean data);
void cms_debug_ns_set_color (const gchar * ns, gboolean color);
void cms_debug_ns_set_timestamp (const gchar * ns, gboolean timestamp);

unsigned cms_debug_ns_get_level (const gchar * ns);
gboolean cms_debug_ns_get_data (const gchar * ns);
gboolean cms_debug_ns_get_color (const gchar * ns);
gboolean cms_debug_ns_get_timestamp (const gchar * ns);

unsigned cms_debug_server_get_level (const gchar * ns);
gboolean cms_debug_server_get_data (const gchar * ns);

unsigned cms_debug_max_get_level (const gchar * ns);
gboolean cms_debug_max_get_data (const gchar * ns);

void cms_debug_print_text (char * out, unsigned outsize,
	const char * file, const char * function, int line, 
	const char * ns, unsigned level, const char * text, ...);
void cms_debug_print_binary (char * out, unsigned outsize, 
	const char * file, const char * function, int line, 
	const char * ns, int length, const void * buffer);

#define cms_ns_print(ns, level, text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	ns, level, text, ##arg)

#define cms_ns_error(ns, text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	ns, CMS_DEBUG_LEVEL_ERROR, text, ##arg)
	
#define cms_ns_warning(ns, text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	ns, CMS_DEBUG_LEVEL_WARNING, text, ##arg)

#define cms_ns_info(ns, text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	ns, CMS_DEBUG_LEVEL_INFO, text, ##arg)

#define cms_ns_debug(ns, text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	ns, CMS_DEBUG_LEVEL_DEBUG, text, ##arg)

#define cms_ns_data(ns, length, buffer) \
	cms_debug_print_binary (NULL, 0, __FILE__, __FUNCTION__,__LINE__, \
	ns, length, buffer)

/* old style macros for retaining compatibility */
#define cms_namespace_error cms_ns_error
#define cms_namespace_warning cms_ns_warning
#define cms_namespace_info cms_ns_info
#define cms_namespace_debug cms_ns_debug
#define cms_namespace_data cms_ns_data

#define cms_print(level,text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	NULL, level, text, ##arg)

#define cms_error(text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg)
	
#define cms_warning(text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	NULL, CMS_DEBUG_LEVEL_WARNING, text, ##arg)

#define cms_info(text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	NULL, CMS_DEBUG_LEVEL_INFO, text, ##arg)

#define cms_debug(text, arg...) \
	cms_debug_print_text (NULL, 0, __FILE__, __FUNCTION__, __LINE__, \
	NULL, CMS_DEBUG_LEVEL_DEBUG, text, ##arg)

#define cms_data(length,buffer) \
	cms_debug_print_binary (NULL, 0, __FILE__, __FUNCTION__,__LINE__, \
	NULL, length, buffer)

G_END_DECLS

#endif /* _CMS_DEBUG_H */
