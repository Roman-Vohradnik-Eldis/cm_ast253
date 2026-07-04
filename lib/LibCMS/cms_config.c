#include "cms_config.h"
#include "cms_debug.h"
#include "cms_misc.h"

#define CMS_CONFIG_MAGIC 0x72060904

#define SECTION		0
#define SUBSECTION	1
#define ITEM		2

int isblank (int c) { return c == ' ' || c == '\t'; }
int endofline (int c) { return c == '\n' || c == '\r'; }

void _cms_config_skip_space (cms_config_t * config, unsigned * pos);
void _cms_config_new_line (cms_config_t * config, unsigned * pos);

gboolean _cms_config_find_section (cms_config_t * config,
	unsigned startpos, const char * name, gboolean exception);
gboolean _cms_config_find_next_section (cms_config_t * config,
	unsigned startpos, char ** name, gboolean exception);
gboolean _cms_config_find_subsection (cms_config_t * config,
	unsigned startpos, const char * name, gboolean exception);
gboolean _cms_config_find_next_subsection (cms_config_t * config,
	unsigned startpos, char ** name, gboolean exception);
gboolean _cms_config_find_item (cms_config_t * config,
	unsigned startpos, const char * name, gboolean exception);
gboolean _cms_config_find_next_item (cms_config_t * config,
	unsigned startpos, char ** name, gboolean exception);

gboolean _cms_config_find_next_value_string (cms_config_t * config,
	char ** value, gboolean exception);
gboolean _cms_config_find_next_value_int (cms_config_t * config,
	int * value, gboolean exception);
gboolean _cms_config_find_next_value_double (cms_config_t * config,
	double * value, gboolean exception);
gboolean _cms_config_find_next_value_bool (cms_config_t * config,
	gboolean * value, gboolean exception);

cms_config_t * cms_config_new (const char * path)
{
	cms_config_t * config;
	int fd = -1;
	struct stat statbuf;
	int status;
	int size, size2;

	config = (cms_config_t *) g_malloc0 (sizeof (cms_config_t));
	if (!config)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}
	
	// zapamatujeme si soubor
	config->config_file = g_strdup (path);
	if (!config->config_file)
	{
		cms_ns_error ("libcms", "g_strdup error");
		goto failed;
	}
	
	// vytvorime stringy
	config->section_name = g_string_new ("");
	config->section_pos = 0;
	config->subsection_name = g_string_new ("");
	config->subsection_pos = 0;
	config->item_name = g_string_new ("");
	config->item_pos = 0;

	// otevreme soubor
	fd = open (path, O_RDONLY);
	if (fd == -1)
	{
		cms_ns_error ("libcms", "open error (%s,%d,%s)", 
			path, errno, strerror (errno));
		goto failed;
	}

	// precteme velikost souboru
	status = fstat (fd, &statbuf);
	if (status == -1 || statbuf.st_size == 0)
	{
		cms_ns_error ("libcms", "stat error (%s,%d,%d,%s)", 
			path, statbuf.st_size, errno, strerror (errno));
		goto failed;
	}

	// nacteme data
	config->data = g_malloc (statbuf.st_size);
	if (!config->data)
	{
		cms_ns_error ("libcms", "g_malloc error");
		goto failed;
	}
	size = 0;
	while (size < statbuf.st_size)
	{
		size2 = read (fd, config->data + size, statbuf.st_size - size);
		if (size2 == -1)
		{
			cms_ns_error ("libcms", "read error (%s,%d,%s)", 
				path, errno, strerror (errno));
			goto failed;
		}
		if (size2 == 0)
		{
			break;
		}
		size += size2;
	}

	config->data_size = size;//statbuf.st_size;

	// zavreme soubor
	status = close (fd);

	config->magic = CMS_CONFIG_MAGIC;
	return config;

failed:
	if (config)
	{
		if (fd != -1)
		{
			close (fd);
		}
		if (config->data)
		{
			g_free (config->data);
		}
		if (config->config_file)
		{
			g_free (config->config_file);
		}
		g_free (config);
	}

	return NULL;
}

cms_config_t * cms_config_new_with_preprocessor (
	const char * path, const char * preprocessor)
{
	cms_config_t * config;
	char * dirname;
	char * basename;
	char * tmpdir;
	char * argv[3];
	int fd = -1;
	int status;
	
	config = (cms_config_t *) g_malloc0 (sizeof (cms_config_t));
	if (!config)
	{
		cms_ns_error ("libcms", "g_malloc0 error");
		goto failed;
	}
	
	// zapamatujeme si soubor
	config->config_file = g_strdup (path);
	if (!config->config_file)
	{
		cms_ns_error ("libcms", "g_strdup error");
		goto failed;
	}
	
	// vytvorime stringy
	config->section_name = g_string_new ("");
	config->section_pos = 0;
	config->subsection_name = g_string_new ("");
	config->subsection_pos = 0;
	config->item_name = g_string_new ("");
	config->item_pos = 0;

	// zkontrolujeme existenci preprocesoru
	if (!preprocessor)
	{
		cms_ns_error ("libcms", "preprocessor is missing (%s)", preprocessor);
		goto failed;
	}

	// zjistime cesty
	dirname = g_path_get_dirname (path);
	basename = g_path_get_basename (path);
	if (dirname[0] != '/')
	{
		tmpdir = g_build_path (G_DIR_SEPARATOR_S, g_getenv ("PWD"), dirname, NULL);
		g_free (dirname);
		dirname = tmpdir;
	}

	// zavolame preprocessor
	argv[0] = (char *) preprocessor;
	argv[1] = basename;
	argv[2] = NULL;
	status = g_spawn_sync (dirname, argv, NULL, G_SPAWN_SEARCH_PATH,
		NULL, NULL, &config->data, NULL, NULL, NULL);
	if (!status)
	{
		cms_ns_error ("libcms", "preprocessor failure (%s)", path);
		goto failed;
	}
	config->data_size = strlen (config->data);
	g_free (dirname);
	g_free (basename);

	config->magic = CMS_CONFIG_MAGIC;
	return config;

failed:
	if (config)
	{
		if (fd != -1)
		{
			close (fd);
		}
		if (config->data)
		{
			g_free (config->data);
		}
		if (config->config_file)
		{
			g_free (config->config_file);
		}
		g_free (config);
	}

	return NULL;
}

void cms_config_delete (cms_config_t * config)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	config->magic = 0;

	if (config->config_file)
	{
		g_free (config->config_file);
	}
	if (config->data)
	{
		g_free (config->data);
	}
	if (config->section_name)
	{
		g_string_free (config->section_name, TRUE);
	}
	if (config->subsection_name)
	{
		g_string_free (config->subsection_name, TRUE);
	}
	if (config->item_name)
	{
		g_string_free (config->item_name, TRUE);
	}
	g_free (config);
}

void cms_config_reset (cms_config_t * config)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	g_string_truncate (config->section_name, 0);
	config->section_pos = 0;
	g_string_truncate (config->subsection_name, 0);
	config->subsection_pos = 0;
	g_string_truncate (config->item_name, 0);
	config->item_pos = 0;
}

void _cms_config_skip_space (cms_config_t * config, unsigned * pos)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	while (*pos < config->data_size)
	{
		if (config->data[*pos] != ' ' && config->data[*pos] != '\t')
		{
			break;
		}
		(*pos)++;
	}
}

void _cms_config_new_line (cms_config_t * config, unsigned * pos)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	while (*pos < config->data_size)
	{
		if (endofline (config->data[(*pos)++]))
		{
			break;
		}
	}
}

gboolean _cms_config_find_section (cms_config_t * config,
	unsigned pos, const char * name, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	// zkontroluji, zda jsem na zacatku radky
	if (pos > 0 && !endofline (config->data[pos - 1]))
	{
newline:
		_cms_config_new_line (config, &pos);
	}

	// konec souboru
	if (pos >= config->data_size)
	{
		goto failed;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] != '[')
	{
		goto newline;
	}
	pos++;

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na retezec
	if (strncasecmp (config->data + pos, name, strlen (name)))
	{
		goto newline;
	}
	pos += strlen (name);

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] != ']')
	{
		goto newline;
	}
	pos++;

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// komentar nebo konec radky
	if (config->data[pos] != '#' && !endofline (config->data[pos]))
	{
		goto error;
	}

	// novy radek
	_cms_config_new_line (config, &pos);

	// zapamatujeme si jmeno a pozici sekce
	g_string_assign (config->section_name, name);
	g_string_truncate (config->subsection_name, 0);
	g_string_truncate (config->item_name, 0);
	config->section_pos = config->subsection_pos = config->item_pos = pos;

	return TRUE;

failed:
	if (exception)
	{
		cms_ns_error ("libcms", "unable to find section %s, file %s", name, config->config_file);
	}
	return FALSE;

error:
	cms_ns_error ("libcms", "configuration file section error (%s)", name);
	return FALSE;
}

gboolean _cms_config_find_next_section (cms_config_t * config,
	unsigned pos, char ** name, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	GString * str = g_string_new ("");
	if (*name)
	{
		g_free (*name);
		*name = NULL;
	}

	// zkontroluji, zda jsem na zacatku radky
	if (pos > 0 && !endofline (config->data[pos - 1]))
	{
newline:
		_cms_config_new_line (config, &pos);
	}

	// konec souboru
	if (pos >= config->data_size)
	{
		goto failed;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] != '[')
	{
		goto newline;
	}
	pos++;

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na retezec
	while (TRUE)
	{
		if (pos >= config->data_size)
		{
			goto error;
		}
		if (config->data[pos] == '#' || endofline (config->data[pos]))
		{
			goto error;
		}
		if (config->data[pos] == ']')
		{
			break;
		}
		g_string_append_c (str, config->data[pos++]);
	}
	while (isblank (str->str[str->len - 1]))
	{
		g_string_truncate (str, str->len - 1);
	}

	// novy radek
	_cms_config_new_line (config, &pos);

	// zapamatujeme si jmeno a pozici sekce
	g_string_assign (config->section_name, str->str);
	g_string_truncate (config->subsection_name, 0);
	g_string_truncate (config->item_name, 0);
	config->section_pos = config->subsection_pos = config->item_pos = pos;

	*name = g_string_free (str, FALSE);
	return TRUE;

failed:
	if (exception)
	{
		cms_ns_error ("libcms", "unable to next find section %s, %s", name, config->config_file);
	}
	*name = g_string_free (str, FALSE);
	return FALSE;

error:
	cms_ns_error ("libcms", "configuration file section error (%s)", name);
	*name = g_string_free (str, FALSE);
	return FALSE;
}

gboolean _cms_config_find_subsection (cms_config_t * config,
	unsigned pos, const char * name, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	// zkontroluji, zda jsem na zacatku radky
	if (pos > 0 && !endofline (config->data[pos - 1]))
	{
newline:
		_cms_config_new_line (config, &pos);
	}

	// konec souboru
	if (pos >= config->data_size)
	{
		goto failed;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] == '[')
	{
		goto failed;
	}

	// test na zavorku
	if (config->data[pos] != '<')
	{
		goto newline;
	}
	pos++;

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na retezec
	if (strncasecmp (config->data + pos, name, strlen (name)))
	{
		goto newline;
	}
	pos += strlen (name);

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] != '>')
	{
		goto newline;
	}
	pos++;

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// komentar nebo konec radky
	if (config->data[pos] != '#' && !endofline (config->data[pos]))
	{
		goto error;
	}

	// novy radek
	_cms_config_new_line (config, &pos);

	// zapamatujeme si nazev a pozici podsekce
	g_string_assign (config->subsection_name, name);
	g_string_truncate (config->item_name, 0);
	config->subsection_pos = config->item_pos = pos;

	return TRUE;

failed:
	if (exception)
	{
		cms_ns_error ("libcms", "unable to find subsection %s, section %s",
			name, config->section_name->str);
	}
	return FALSE;

error:
	cms_ns_error ("libcms", "configuration file subsection error %s, section %s",
		name, config->section_name->str);
	return FALSE;
}

gboolean _cms_config_find_next_subsection (cms_config_t * config,
	unsigned pos, char ** name, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	GString * str = g_string_new ("");
	if (*name)
	{
		g_free (*name);
		*name = NULL;
	}

	// zkontroluji, zda jsem na zacatku radky
	if (pos > 0 && !endofline (config->data[pos - 1]))
	{
newline:
		_cms_config_new_line (config, &pos);
	}

	// konec souboru
	if (pos >= config->data_size)
	{
		goto failed;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] == '[')
	{
		goto failed;
	}

	// test na zavorku
	if (config->data[pos] != '<')
	{
		goto newline;
	}
	pos++;

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na retezec
	while (TRUE)
	{
		if (pos >= config->data_size)
		{
			goto error;
		}
		if (config->data[pos] == '#' || endofline (config->data[pos]))
		{
			goto error;
		}
		if (config->data[pos] == '>')
		{
			break;
		}
		g_string_append_c (str, config->data[pos++]);
	}

	while (str->len > 0 && isblank (str->str[str->len - 1]))
	{
		g_string_truncate (str, str->len - 1);
	}

	// novy radek
	_cms_config_new_line (config, &pos);

	// zapamatujeme si nazev a pozici podsekce
	g_string_assign (config->subsection_name, str->str);
	g_string_truncate (config->item_name, 0);
	config->subsection_pos = config->item_pos = pos;

	*name = g_string_free (str, FALSE);
	return TRUE;

failed:
	if (exception)
	{
		cms_ns_error ("libcms", "unable to find subsection %s, section %s",
			name, config->section_name->str);
	}
	*name = g_string_free (str, FALSE);
	return FALSE;

error:
	cms_ns_error ("libcms", "configuration file subsection error %s, section %s",
		name, config->section_name->str);
	*name = g_string_free (str, FALSE);
	return FALSE;
}

gboolean _cms_config_find_item (cms_config_t * config,
	unsigned pos, const char * name, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	// zkontroluji, zda jsem na zacatku radky
	if (pos > 0 && !endofline (config->data[pos - 1]))
	{
newline:
		_cms_config_new_line (config, &pos);
	}

	// konec souboru
	if (pos >= config->data_size)
	{
		goto failed;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] == '[' || config->data[pos] == '<')
	{
		goto failed;
	}

	// test na retezec
	if (strncasecmp (config->data + pos, name, strlen (name)))
	{
		goto newline;
	}
	pos += strlen (name);

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na rovnitko
	if (config->data[pos] != '=')
	{
		goto newline;
	}
	pos++;

	// zapamatujeme si nazev a pozici polozky
	g_string_assign (config->item_name, name);
	config->item_pos = pos;

	return TRUE;

failed:
	if (exception)
	{
		cms_ns_error ("libcms", "unable to find item %s, subsection %s, section %s",
			name, config->subsection_name->str, config->section_name->str);
	}
	return FALSE;
}

gboolean _cms_config_find_next_item (cms_config_t * config,
	unsigned pos, char ** name, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	GString * str = g_string_new ("");
	if (*name)
	{
		g_free (*name);
		name = NULL;
	}
		

	// zkontroluji, zda jsem na zacatku radky
	if (pos > 0 && !endofline (config->data[pos - 1]))
	{
newline:
		_cms_config_new_line (config, &pos);
	}

	// konec souboru
	if (pos >= config->data_size)
	{
		goto failed;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na zavorku
	if (config->data[pos] == '[' || config->data[pos] == '<')
	{
		goto failed;
	}

	// test na retezec
	while (g_ascii_isalnum (config->data[pos])
		|| config->data[pos] == '-'
		|| config->data[pos] == '_'
		|| config->data[pos] == '/')
	{
		g_string_append_c (str, config->data[pos++]);
	}

	// preskocime mezery
	_cms_config_skip_space (config, &pos);

	// test na rovnitko
	if (config->data[pos] != '=')
	{
		goto newline;
	}
	pos++;

	// zapamatujeme si nazev a pozici polozky
	g_string_assign (config->item_name, str->str);
	config->item_pos = pos;

	*name = g_string_free (str, FALSE);
	return TRUE;

failed:
	if (exception)
	{
		cms_ns_error ("libcms", "unable to find item %s, subsection %s, section %s",
			name, config->subsection_name->str, config->section_name->str);
	}
	*name = g_string_free (str, FALSE);
	return FALSE;
}

gboolean _cms_config_find_next_value_string (cms_config_t * config,
	char ** value, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	GString * str = g_string_new ("");
	if (*value)
	{
		g_free (*value);
		*value = NULL;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &config->item_pos);

	// nasteme retezec
	while (!endofline (config->data[config->item_pos])
		&& config->data[config->item_pos] != ','
		&& config->data[config->item_pos] != '#'
		&& config->item_pos < config->data_size)
	{
		g_string_append_c (str, config->data[config->item_pos++]);
	}

	// preskocime carku
	if (config->item_pos < config->data_size
		&& config->data[config->item_pos] == ',')
	{
		config->item_pos++;
		while (config->item_pos < config->data_size)
		{
			if (config->data[config->item_pos] != ' ' 
				&& config->data[config->item_pos] != '\t'
				&& config->data[config->item_pos] != '\n')
			{
				break;
			}
			config->item_pos++;
		}
	}

	// smazeme prazdne znaky
	while (str->len > 0 && isblank (str->str[str->len - 1]))
	{
		g_string_truncate (str, str->len - 1);
	}

	*value = g_string_free (str, FALSE);

	if (!**value)
	{
		if (*value)
		{
			g_free (*value);
			*value = NULL;
		}
		
		// retezec je prazny
		if (exception)
		{
			cms_ns_error ("libcms", "unable to read value of item %s, "
				"subsection %s, section %s", config->item_name->str, 
				config->subsection_name->str, config->section_name->str);
		}
		
		return FALSE;
	}

	return TRUE;
}

gboolean _cms_config_find_line (cms_config_t * config,
	char ** value, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	GString * str = g_string_new ("");
	if (*value)
	{
		g_free (*value);
		*value = NULL;
	}

	// preskocime mezery
	_cms_config_skip_space (config, &config->item_pos);

	// nasteme retezec
	while (!endofline (config->data[config->item_pos])
		&& config->data[config->item_pos] != '#'
		&& config->item_pos < config->data_size)
	{
		g_string_append_c (str, config->data[config->item_pos++]);
	}

	*value = g_string_free (str, FALSE);

	if (!**value)
	{
		if (*value)
		{
			g_free (*value);
			*value = NULL;
		}

		// retezec je prazny
		if (exception)
		{
			cms_ns_error ("libcms", "unable to read value of item %s, "
				"subsection %s, section %s", config->item_name->str,
				config->subsection_name->str, config->section_name->str);
		}

		return FALSE;
	}

	return TRUE;
}

gboolean _cms_config_find_next_value_int (cms_config_t * config,
	int * value, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	char * str = NULL;
	char * ptr;

	*value = 0;

	if (!_cms_config_find_next_value_string (config, &str, exception))
	{
		return FALSE;
	}

	if (str[0] == '0' && str[1] == 'x')
	{
		*value = strtol (str + 2, &ptr, 16);
	}
	else
	{
		*value = strtol (str, &ptr, 10);
	}

	if (!ptr || *ptr)
	{
		if (exception)
		{
			cms_ns_error ("libcms", "unable to read value of item %s, "
				"subsection %s, section %s", config->item_name->str, 
				config->subsection_name->str, config->section_name->str);
		}
		if (str)
		{
			g_free (str);
		}
		return FALSE;
	}

	if (str)
	{
		g_free (str);
	}
	return TRUE;
}

gboolean _cms_config_find_next_value_double (cms_config_t * config,
	double * value, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	char * str = NULL;
	char * ptr;

	*value = 0;

	if (!_cms_config_find_next_value_string (config, &str, exception))
	{
		return FALSE;
	}

	*value = g_ascii_strtod (str, &ptr);

	if (!ptr || *ptr)
	{
		if (exception)
		{
			cms_ns_error ("libcms", "unable to read value of item %s, "
				"subsection %s, section %s", config->item_name->str, 
				config->subsection_name->str, config->section_name->str);
		}
		if (str)
		{
			g_free (str);
		}
		return FALSE;
	}

	if (str)
	{
		g_free (str);
	}
	return TRUE;
}

gboolean _cms_config_find_next_value_bool (cms_config_t * config,
	gboolean * value, gboolean exception)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	char * str = NULL;

	*value = FALSE;

	if (!_cms_config_find_next_value_string (config, &str, exception))
	{
		return FALSE;
	}

	if (!strcasecmp (str, "yes")
		|| !strcasecmp (str, "on")
		|| !strcasecmp (str, "TRUE"))
	{
		*value = TRUE;
	}

	else if (!strcasecmp (str, "no")
		|| !strcasecmp (str, "off")
		|| !strcasecmp (str, "FALSE"))
	{
		*value = FALSE;
	}

	else
	{
		if (exception)
		{
			cms_ns_error ("libcms", "unable to read value of item %s, "
				"subsection %s, section %s", config->item_name->str, 
				config->subsection_name->str, config->section_name->str);
		}
		if (str)
		{
			g_free (str);
		}
		return FALSE;
	}

	if (str)
	{
		g_free (str);
	}
	return TRUE;
}

void cms_config_find_section (cms_config_t * config, const char * name, gboolean first)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (!_cms_config_find_section (config, first ? 0 : config->section_pos, name, TRUE))
	{
		exit (-1);
	}
}

void cms_config_find_next_section (cms_config_t * config, char ** name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (!_cms_config_find_next_section (config, config->section_pos, name, TRUE))
	{
		exit (-1);
	}
}

gboolean cms_config_find_section_2 (cms_config_t * config, const char * name, gboolean first)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	return _cms_config_find_section (config, first ? 0 : config->section_pos, name, FALSE);
}

gboolean cms_config_find_next_section_2 (cms_config_t * config, char ** name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	return _cms_config_find_next_section (config, config->section_pos, name, FALSE);
}

void cms_config_find_subsection (cms_config_t * config, const char * name, gboolean first)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (!_cms_config_find_subsection (config, first ? config->section_pos 
		: config->subsection_pos, name, TRUE))
	{
		exit (-1);
	}
}

void cms_config_find_next_subsection (cms_config_t * config, char ** name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (!_cms_config_find_next_subsection (config, config->subsection_pos, name, TRUE))
	{
		exit (-1);
	}
}

gboolean cms_config_find_subsection_2 (cms_config_t * config, const char * name, gboolean first)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	return _cms_config_find_subsection (config, first ? config->section_pos 
		: config->subsection_pos, name, FALSE);
}

gboolean cms_config_find_next_subsection_2 (cms_config_t * config, char ** name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	return _cms_config_find_next_subsection (config, config->subsection_pos, name, FALSE);
}

void cms_config_find_item (cms_config_t * config, const char * name, gboolean first)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (!_cms_config_find_item (config, first ? config->subsection_pos 
		: config->item_pos, name, TRUE))
	{
		exit (-1);
	}
}

void cms_config_find_next_item (cms_config_t * config, char ** name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (!_cms_config_find_next_item (config, config->item_pos, name, TRUE))
	{
		exit (-1);
	}
}

gboolean cms_config_find_item_2 (cms_config_t * config, const char * name, gboolean first)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	return _cms_config_find_item (config, first ? config->subsection_pos 
		: config->item_pos, name, FALSE);
}

gboolean cms_config_find_next_item_2 (cms_config_t * config, char ** name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	return _cms_config_find_next_item (config, config->item_pos, name, FALSE);
}

void cms_config_find_value_string (cms_config_t * config, char ** value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	if (!_cms_config_find_next_value_string (config, value, TRUE))
	{
		exit (-1);
	}
}

void cms_config_find_value_int (cms_config_t * config, int * value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	if (!_cms_config_find_next_value_int (config, value, TRUE))
	{
		exit (-1);
	}
}

void cms_config_find_value_double (cms_config_t * config, double * value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	if (!_cms_config_find_next_value_double (config, value, TRUE))
	{
		exit (-1);
	}
}

void cms_config_find_value_bool (cms_config_t * config, gboolean * value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	if (!_cms_config_find_next_value_bool (config, value, TRUE))
	{
		exit (-1);
	}
}

gboolean cms_config_find_value_string_2 (cms_config_t * config, char ** value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	return _cms_config_find_next_value_string (config, value, FALSE);
}

gboolean cms_config_find_value_int_2 (cms_config_t * config, int * value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	return _cms_config_find_next_value_int (config, value, FALSE);
}

gboolean cms_config_find_value_double_2 (cms_config_t * config, double * value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	return _cms_config_find_next_value_double (config, value, FALSE);
}

gboolean cms_config_find_value_bool_2 (cms_config_t * config, gboolean * value, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	
	return _cms_config_find_next_value_bool (config, value, FALSE);
}

char * cms_config_get_value_string (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	char * value = NULL;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_string (config, &value, TRUE))
	{
		exit (-1);
	}
	
	return value;
}

int cms_config_get_value_int (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	int value;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_int (config, &value, TRUE))
	{
		exit (-1);
	}
	
	return value;
}
double cms_config_get_value_double (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	double value;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_double (config, &value, TRUE))
	{
		exit (-1);
	}
	
	return value;
}
gboolean cms_config_get_value_bool (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	gboolean value;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_bool (config, &value, TRUE))
	{
		exit (-1);
	}
	
	return value;
}

char * cms_config_get_value_string_2 (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	char * value = NULL;
	
	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_string (config, &value, FALSE))
	{
		return NULL;
	}
	
	return value;
}

char * cms_config_get_line_2 (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	char * value = NULL;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_line (config, &value, FALSE))
	{
		return NULL;
	}

	return value;
}

int cms_config_get_value_int_2 (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	int value;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_int (config, &value, FALSE))
	{
		return 0;
	}
	
	return value;
}

double cms_config_get_value_double_2 (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	double value;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_double (config, &value, FALSE))
	{
		return 0;
	}
	
	return value;
}

gboolean cms_config_get_value_bool_2 (cms_config_t * config, const char * name)
{
	if (!config || config->magic != CMS_CONFIG_MAGIC)
	{
		cms_ns_error ("libcms", "wrong magic %08X", config->magic);
		return 0;
	}

	gboolean value;

	if (name)
	{
		cms_config_find_item (config, name, TRUE);
	}
	if (!_cms_config_find_next_value_bool (config, &value, FALSE))
	{
		return FALSE;
	}
	
	return value;
}

