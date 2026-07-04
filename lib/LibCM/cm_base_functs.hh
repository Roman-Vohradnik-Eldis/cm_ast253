#ifndef CM_BASE_FUNCTS_H
#define CM_BASE_FUNCTS_H

#include "cm_header_internal.hh"
#include <fnmatch.h>
#include <list>
#include <string>

#define CM_FILTER_ONLYSTARS_UNLIMITED 2

// #define cms_ns_if_print(ns,level,arg...); do { if (cms_debug_max_get_level(ns) >= level) cms_ns_print(ns,level, ##arg); } while (0)
// #define cms_ns_if_print(ns,level,arg...) { if (cms_debug_max_get_level(ns) >= level) cms_ns_print(ns,level, ##arg); }
//#define cms_ns_if_print(ns,level,arg...) {cms_ns_print(ns,level, ##arg); }

double      libCM_GetActualTimeDouble();
long double libCM_GetActualTimeLongDouble();
std::string libCM_TimeToString(double T, int variant = 0);
bool        libCM_isNameMatching(const char *pattern, const char *name);
bool        libCM_isNameMatching(std::string pattern, std::string name);

unsigned    libCM_IndexArray_add_smart(std::list <  std::list<uint16_t> > *index_array_dst, std::list<uint16_t> *index_array_src_item,
                                    bool *changed = NULL, std::string *debug_name=NULL); // returning ErrorCode
unsigned    libCM_IndexArray_add_smart(std::list <  std::list<uint16_t> > *index_array_dst, std::list <  std::list<uint16_t> > *index_array_src,
                                    bool *changed = NULL, std::string *debug_name=NULL); // returning ErrorCode
bool        libCM_test_EventFields_by_IndexArray(unsigned array_y, unsigned array_x, std::list <unsigned> const *EventFields, std::list <  std::list<uint16_t> > *index_array);
bool        libCM_reduce_EventFields_by_IndexArray(unsigned array_y, unsigned array_x, std::list <unsigned> *EventFields, std::list <  std::list<uint16_t> > *index_array); // retval = reduced any
void        libCM_CreateDirectoryFromPath(const gchar *path);
std::string libCM_GetProgramPath(void);
std::string libCM_GetLastNameFromPath(std::string path);


#endif // CM_BASE_FUNCTS_H
