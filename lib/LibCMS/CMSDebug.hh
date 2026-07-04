
#ifndef _CMS_DEBUG_HH
#define _CMS_DEBUG_HH

#include "CMSHeader.hh"
#include "cms_debug.h"

#define CMSDebugInit(x) cms_debug_init ((x).c_str ())

#define cms_debug_set_error(x) if (x) {  \
	if (cms_debug_ns_get_level (NULL) < CMS_DEBUG_LEVEL_ERROR) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_ERROR); \
} else { \
	if (cms_debug_ns_get_level (NULL) >= CMS_DEBUG_LEVEL_ERROR) \
		cms_debug_ns_set_level (NULL, 0); \
}

#define cms_debug_set_warning(x) if (x) {  \
	if (cms_debug_ns_get_level (NULL) < CMS_DEBUG_LEVEL_WARNING) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_WARNING); \
} else { \
	if (cms_debug_ns_get_level (NULL) >= CMS_DEBUG_LEVEL_WARNING) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_ERROR); \
}

#define cms_debug_set_info(x) if (x) {  \
	if (cms_debug_ns_get_level (NULL) < CMS_DEBUG_LEVEL_INFO) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_INFO); \
} else { \
	if (cms_debug_ns_get_level (NULL) >= CMS_DEBUG_LEVEL_INFO) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_WARNING); \
}

#define cms_debug_set_debug(x) if (x) {  \
	if (cms_debug_ns_get_level (NULL) < CMS_DEBUG_LEVEL_DEBUG) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_DEBUG); \
} else { \
	if (cms_debug_ns_get_level (NULL) >= CMS_DEBUG_LEVEL_DEBUG) \
		cms_debug_ns_set_level (NULL, CMS_DEBUG_LEVEL_INFO); \
}

#define cms_debug_set_data(x) cms_debug_ns_set_data(NULL,x)
#define cms_debug_set_timestamp(x) cms_debug_ns_set_timestamp(NULL,x)
#define cms_debug_set_color(x) cms_debug_ns_set_color(NULL,x)

#define CONFIG_ERROR(text, arg...) { \
	char out[0x10000]; \
	cms_debug_print_text (out, sizeof (out), \
		__FILE__, __FUNCTION__, __LINE__,  \
		NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg); \
	throw std::runtime_error (std::string ("CONFIG ") + std::string (out)); \
}

#define NET_ERROR(text, arg...) { \
	char out[0x10000]; \
	cms_debug_print_text (out, sizeof (out), \
		__FILE__, __FUNCTION__, __LINE__, \
		NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg); \
	throw std::runtime_error (std::string ("NET ") + std::string (out)); \
}

#define DISK_ERROR(text, arg...) { \
	char out[0x10000]; \
	cms_debug_print_text (out, sizeof (out), \
		__FILE__, __FUNCTION__, __LINE__, \
		NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg); \
	throw std::runtime_error (std::string ("DISK ") + std::string (out)); \
}

#define DRIVE_ERROR(text, arg...) { \
	char out[0x10000]; \
	cms_debug_print_text (out, sizeof (out), \
		__FILE__, __FUNCTION__, __LINE__, \
		NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg); \
	throw std::runtime_error (std::string ("DRIVE ") + std::string (out)); \
}

#define SYSTEM_ERROR(text, arg...) { \
	char out[0x10000]; \
	cms_debug_print_text (out, sizeof (out), \
		__FILE__, __FUNCTION__, __LINE__, \
		NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg); \
	throw std::runtime_error (std::string ("SYSTEM ") + std::string (out)); \
}

#define INTERNAL_ERROR(text, arg...) { \
	char out[0x10000]; \
	cms_debug_print_text (out, sizeof (out), \
		__FILE__, __FUNCTION__, __LINE__, \
		NULL, CMS_DEBUG_LEVEL_ERROR, text, ##arg); \
	throw std::runtime_error (std::string ("INTERNAL ") + std::string (out)); \
}

/* macros for compatibility with LibCMS4 */
#undef ERROR
#define ERROR(ns,text,arg...) cms_ns_error(ns,text,##arg)
#undef WARNING
#define WARNING(ns,text,arg...) cms_ns_warning(ns,text,##arg)
#undef INFO
#define INFO(ns,text,arg...) cms_ns_info(ns,text,##arg)
#undef DEBUG
#define DEBUG(ns,text,arg...) cms_ns_debug(ns,text,##arg)
#undef DATA
#define DATA(ns,data,length) cms_ns_data(ns,length,data)

#endif /* _CMS_DEBUG_HH */
