#ifndef _CMS_CONFIG_HH
#define _CMS_CONFIG_HH

#include "CMSHeader.hh"
#include "CMSDebug.hh"
#include "cms_config.h"

class CMSConfig
{
	cms_config_t * config;
	
  public:
	CMSConfig (std::string path)
	{
		config = cms_config_new (path.c_str ());
		if (!config) throw std::runtime_error ("config init error");
	}
	CMSConfig (std::string path, std::string preprocessor)
	{
		config = cms_config_new_with_preprocessor (path.c_str (), preprocessor.c_str ());
		if (!config) throw std::runtime_error ("config init error");
	}
	~CMSConfig ()
	{
		cms_config_delete (config);
	}

	void Reset()
	{
		cms_config_reset (config);
	}


	void FindSection (std::string name, bool first = TRUE)
	{
		cms_config_find_section (config, name.c_str (), first);
	}
	
	void FindNextSection (std::string & name)
	{
		char * str = NULL;
		cms_config_find_next_section (config, &str);
		if (str) name.assign (str); else name.clear ();
		g_free (str);
	}
	
	bool FindSection2 (std::string name, bool first = TRUE)
	{
		return cms_config_find_section_2 (config, name.c_str (), first);
	}
	
	bool FindNextSection2 (std::string & name)
	{
		char * str = NULL;
		bool status = cms_config_find_next_section_2 (config, &str);
		if (str) name.assign (str); else name.clear ();
		g_free (str);
		return status;
	}


	void FindSubsection (std::string name, bool first = TRUE)
	{
		cms_config_find_subsection (config, name.c_str (), first);
	}
	
	void FindNextSubsection (std::string & name)
	{
		char * str = NULL;
		cms_config_find_next_subsection (config, &str);
		if (str) name.assign (str); else name.clear ();
		g_free (str);
	}
	
	bool FindSubsection2 (std::string name, bool first = TRUE)
	{
		return cms_config_find_subsection_2 (config, name.c_str (), first);
	}
	
	bool FindNextSubsection2 (std::string & name)
	{
		char * str = NULL;
		bool status = cms_config_find_next_subsection_2 (config, &str);
		if (str) name.assign (str); else name.clear ();
		g_free (str);
		return status;
	}


	void FindItem (std::string name, bool first = TRUE)
	{
		cms_config_find_item (config, name.c_str (), first);
	}
	
	void FindNextItem (std::string & name)
	{
		char * str = NULL;
		cms_config_find_next_item (config, &str);
		if (str) name.assign (str); else name.clear ();
		g_free (str);
	}
	
	bool FindItem2 (std::string name, bool first = TRUE)
	{
		return cms_config_find_item_2 (config, name.c_str (), first);
	}
	
	bool FindNextItem2 (std::string & name)
	{
		char * str = NULL;
		bool status = cms_config_find_next_item_2 (config, &str);
		if (str) name.assign (str); else name.clear ();
		g_free (str);
		return status;
	}


	void FindValueString (std::string & value)
	{
		char * str = NULL;
		cms_config_find_value_string (config, &str, NULL);
		if (str) value.assign (str); else value.clear ();
		g_free (str);
	}
	
	void FindValueInt (int & value)
	{
		cms_config_find_value_int (config, &value, NULL);
	}
	
	void FindValueDouble (double & value)
	{
		cms_config_find_value_double (config, &value, NULL);
	}
	
	void FindValueBool (bool & value)
	{
		gboolean b;
		cms_config_find_value_bool (config, &b, NULL);
		value = b;
	}

	
	bool FindValueString2 (std::string & value)
	{
		char * str = NULL;
		bool status = cms_config_find_value_string_2 (config, &str, NULL);
		if (str) value.assign (str); else value.clear ();
		g_free (str);
		return status;
	}
	
	bool FindValueInt2 (int & value)
	{
		return cms_config_find_value_int_2 (config, &value, NULL);
	}
	
	bool FindValueDouble2 (double & value)
	{
		return cms_config_find_value_double_2 (config, &value, NULL);
	}
	
	bool FindValueBool2 (bool & value)
	{
		gboolean b;
		bool status = cms_config_find_value_bool_2 (config, &b, NULL);
		value = b;
		return status;
	}


	void FindValueString (std::string & value, std::string name)
	{
		char * str = NULL;
		cms_config_find_value_string (config, &str, name.c_str ());
		if (str) value.assign (str); else value.clear ();
		g_free (str);
	}

	void FindValueInt (int & value, std::string name)
	{
		cms_config_find_value_int (config, &value, name.c_str ());
	}

	void FindValueDouble (double & value, std::string name)
	{
		cms_config_find_value_double (config, &value, name.c_str ());
	}

	void FindValueBool (bool & value, std::string name)
	{
		gboolean b;
		cms_config_find_value_bool (config, &b, name.c_str ());
		value = b;
	}

	bool FindValueString2 (std::string & value, std::string name)
	{
		char * str = NULL;
		bool status = cms_config_find_value_string_2 (config, &str, name.c_str ());
		if (str) value.assign (str); else value.clear ();
		g_free (str);
		return status;
	}

	bool FindValueInt2 (int & value, std::string name)
	{
		return cms_config_find_value_int_2 (config, &value, name.c_str ());
	}

	bool FindValueDouble2 (double & value, std::string name)
	{
		return cms_config_find_value_double_2 (config, &value, name.c_str ());
	}

	bool FindValueBool2 (bool & value, std::string name)
	{
		gboolean b;
		bool status = cms_config_find_value_bool_2 (config, &b, name.c_str ());
		value = b;
		return status;
	}

	std::string GetValueString ()
	{
		char * str = cms_config_get_value_string (config, NULL);
		std::string value;
		if (str) value.assign (str);
		g_free (str);
		return value;
	}		

	int GetValueInt ()
	{
		return cms_config_get_value_int (config, NULL);
	}

	double GetValueDouble ()
	{
		return cms_config_get_value_double (config, NULL);
	}

	bool GetValueBool ()
	{
		return cms_config_get_value_bool (config, NULL);
	}

	std::string GetValueString2 ()
	{
		char * str = cms_config_get_value_string_2 (config, NULL);
		std::string value;
		if (str) value.assign (str);
		g_free (str);
		return value;
	}		

	int GetValueInt2 ()
	{
		return cms_config_get_value_int_2 (config, NULL);
	}

	double GetValueDouble2 ()
	{
		return cms_config_get_value_double_2 (config, NULL);
	}

	bool GetValueBool2 ()
	{
		return cms_config_get_value_bool_2 (config, NULL);
	}


	std::string GetValueString (std::string name)
	{
		char * str = cms_config_get_value_string (config, name.c_str ());
		std::string value;
		if (str) value.assign (str);
		g_free (str);
		return value;
	}		

	int GetValueInt (std::string name)
	{
		return cms_config_get_value_int (config, name.c_str ());
	}

	double GetValueDouble (std::string name)
	{
		return cms_config_get_value_double (config, name.c_str ());
	}

	bool GetValueBool (std::string name)
	{
		return cms_config_get_value_bool (config, name.c_str ());
	}

	std::string GetValueString2 (std::string name)
	{
		char * str = cms_config_get_value_string_2 (config, name.c_str ());
		std::string value;
		if (str) value.assign (str);
		g_free (str);
		return value;
	}		

	std::string GetLine2 (std::string name)
	{
		char * str = cms_config_get_line_2 (config, name.c_str ());
		std::string value;
		if (str) value.assign (str);
		g_free (str);
		return value;
	}

	std::string GetLine2 ()
	{
		char * str = cms_config_get_line_2 (config, NULL);
		std::string value;
		if (str) value.assign (str);
		g_free (str);
		return value;
	}

	int GetValueInt2 (std::string name)
	{
		return cms_config_get_value_int_2 (config, name.c_str ());
	}

	double GetValueDouble2 (std::string name)
	{
		return cms_config_get_value_double_2 (config, name.c_str ());
	}

	bool GetValueBool2 (std::string name)
	{
		return cms_config_get_value_bool_2 (config, name.c_str ());
	}
};

#endif /* _CMS_CONFIG_HH */
