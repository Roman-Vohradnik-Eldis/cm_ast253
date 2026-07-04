#ifndef _CMS_CONFIG_H
#define _CMS_CONFIG_H

#include "cms_header.h"

G_BEGIN_DECLS

struct cms_config_s
{
	unsigned magic;
	char * config_file;
	char * data;
	unsigned data_size;
	GString * section_name;
	unsigned section_pos;
	GString * subsection_name;
	unsigned subsection_pos;
	GString * item_name;
	unsigned item_pos;
};

typedef struct cms_config_s cms_config_t;

cms_config_t * cms_config_new (const char * path);
cms_config_t * cms_config_new_with_preprocessor (
	const char * path, const char * preprocessor);
void cms_config_delete (cms_config_t * config);

void cms_config_reset (cms_config_t * config);

void cms_config_find_section (cms_config_t * config, const char * name, gboolean first);
void cms_config_find_next_section (cms_config_t * config, char ** name);
gboolean cms_config_find_section_2 (cms_config_t * config, const char * name, gboolean first);
gboolean cms_config_find_next_section_2 (cms_config_t * config, char ** name);

void cms_config_find_subsection (cms_config_t * config, const char * name, gboolean first);
void cms_config_find_next_subsection (cms_config_t * config, char ** name);
gboolean cms_config_find_subsection_2 (cms_config_t * config, const char * name, gboolean first);
gboolean cms_config_find_next_subsection_2 (cms_config_t * config, char ** name);

void cms_config_find_item (cms_config_t * config, const char * name, gboolean first);
void cms_config_find_next_item (cms_config_t * config, char ** name);
gboolean cms_config_find_item_2 (cms_config_t * config, const char * name, gboolean first);
gboolean cms_config_find_next_item_2 (cms_config_t * config, char ** name);

void cms_config_find_value_string (cms_config_t * config, char ** value, const char * name);
void cms_config_find_value_int (cms_config_t * config, int * value, const char * name);
void cms_config_find_value_double (cms_config_t * config, double * value, const char * name);
void cms_config_find_value_bool (cms_config_t * config, gboolean * value, const char * name);

gboolean cms_config_find_value_string_2 (cms_config_t * config, char ** value, const char * name);
gboolean cms_config_find_value_int_2 (cms_config_t * config, int * value, const char * name);
gboolean cms_config_find_value_double_2 (cms_config_t * config, double * value, const char * name);
gboolean cms_config_find_value_bool_2 (cms_config_t * config, gboolean * value, const char * name);

char * cms_config_get_value_string (cms_config_t * config, const char * name);
int cms_config_get_value_int (cms_config_t * config, const char * name);
double cms_config_get_value_double (cms_config_t * config, const char * name);
gboolean cms_config_get_value_bool (cms_config_t * config, const char * name);

char * cms_config_get_value_string_2 (cms_config_t * config, const char * name);
int cms_config_get_value_int_2 (cms_config_t * config, const char * name);
double cms_config_get_value_double_2 (cms_config_t * config, const char * name);
gboolean cms_config_get_value_bool_2 (cms_config_t * config, const char * name);

char * cms_config_get_line_2 (cms_config_t * config, const char * name);

G_END_DECLS

#endif /* _CMS_CONFIG_H */
