#include "cm_base_functs.hh"
#include "cm_debug.hh"
#include <fnmatch.h>

double libCM_GetActualTimeDouble()
{
  struct timeval  tv;
  gettimeofday (&tv, NULL);
  return (double)tv.tv_sec + (((double)tv.tv_usec) / 1000000.);
}

long double libCM_GetActualTimeLongDouble()
{
  struct timeval  tv;
  gettimeofday (&tv, NULL);
  return (long double)tv.tv_sec + (((long double)tv.tv_usec) / 1000000.);
}

std::string libCM_TimeToString(double T, int variant)
{
  int      ActITime =  T;
  unsigned ActUTime = (T - (double)ActITime) * 1000000.;
  unsigned Y, M, D, h, m, s;
  time2ymdhms (ActITime, &Y, &M, &D, &h, &m, &s);
  char ActTimeStr1s[200];

       if (variant==1) sprintf (ActTimeStr1s, "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u", Y, M, D, h, m, s, (int)(ActUTime/1000));
  else if (variant==2) sprintf (ActTimeStr1s, "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u", Y, M, D, h, m, s);
  else if (variant==3) sprintf (ActTimeStr1s, "%2.2u:%2.2u:%2.2u.%3.3u", h, m, s, (int)(ActUTime/1000));
  else if (variant==4) sprintf (ActTimeStr1s, "%2.2u:%2.2u:%2.2u", h, m, s);
  else if (variant==5) sprintf (ActTimeStr1s, "%2.2u:%2.2u", h, m);
  else                 sprintf (ActTimeStr1s, "%2.2u:%2.2u:%2.2u.%3.3u", h, m, s, (int)(ActUTime/1000)); // default variant 0 same with 3

  return std::string(ActTimeStr1s);
}

bool libCM_isNameMatching(const char *pattern, const char *name)
{
   //FNM_FILE_NAME | FNM_PATHNAME | FNM_PERIOD | FNM_NOESCAPE | FNM_LEADING_DIR | FNM_CASEFOLD | FNM_EXTMATCH
//  int retval =  fnmatch("**", "TxG_A__STATUS[12/23", 0);
  int retval =  fnmatch(pattern, name, 0);
// debug_printf("retval = %s",(retval ? "not matching" : "MATCHING!"));
  return (retval ? false : true );
}
bool libCM_isNameMatching(std::string pattern, std::string name)
{
// debug_printf("retval = %s",(libCM_isNameMatching("**", "TxG_A__STATUS[12/23") ? "MATCHING!" : "not matching"));
  return libCM_isNameMatching(pattern.c_str(), name.c_str());
}



unsigned libCM_IndexArray_add_smart(std::list <  std::list<uint16_t> > *index_array_dst, std::list<uint16_t> *index_array_src_item,
                                    bool *changed, std::string *debug_name) // returning ErrorCode
{
  unsigned ErrorCode = 0;
  if (changed != NULL) *changed = false;
  if ((index_array_dst==NULL)||(index_array_src_item==NULL)) return 4;
  unsigned dimensions = index_array_src_item->size();
  if (index_array_dst->size()>0)
  {
    if (index_array_dst->begin()->size() != dimensions)
    {
      std::string name="?";
      if (debug_name != NULL) name = *debug_name;
      cms_ns_if_print("libcm",1, "\"%s\" - cannot add index array with different dimensions %zu != %d",name.c_str(),index_array_dst->begin()->size(),dimensions);
      return 1;
    }
  }
  if ((dimensions>0)&&(dimensions<=2))
  {
    bool found_same = false;
    for (std::list <  std::list<uint16_t> >::iterator it_dst_idx = index_array_dst->begin(); it_dst_idx != index_array_dst->end(); it_dst_idx++)
    {
      if ((*it_dst_idx) == (*index_array_src_item)) found_same = true; // skutecne funguje takoveto porovnani???
    }
    if (!found_same)
    {
      index_array_dst->push_back( *index_array_src_item );
      index_array_dst->sort();
      if (changed != NULL) *changed = true;
    }
  } else {
    std::string name="?";
    if (debug_name != NULL) name = *debug_name;
    cms_ns_if_print("libcm",1, "\"%s\" - cannot make array with %d dimensions",name.c_str(),dimensions);
    ErrorCode = 2;
  }
  return ErrorCode;
}

unsigned libCM_IndexArray_add_smart(std::list <  std::list<uint16_t> > *index_array_dst, std::list <  std::list<uint16_t> > *index_array_src,
                                    bool *changed, std::string *debug_name) // returning ErrorCode
{
  unsigned ErrorCode = 0;
  if (changed != NULL) *changed = false;
  if ((index_array_dst==NULL)||(index_array_src==NULL)) return 4;
  if ((index_array_dst->size()>0)&&(index_array_src->size()>0))
  {
    for (std::list <  std::list<uint16_t> >::iterator it_src_idx = index_array_src->begin(); (it_src_idx != index_array_src->end()) && (!ErrorCode); it_src_idx++)
    {
      bool local_changed = false;
      unsigned Ecode = libCM_IndexArray_add_smart(index_array_dst, &(*it_src_idx), changed, debug_name);
      if (Ecode) ErrorCode = Ecode;
      if ((local_changed)&&(changed != NULL)) *changed = true;
    }
  }
  else
  {
    if ((index_array_src->size()>0)&&(changed != NULL))
	*changed = true;
    index_array_src->clear();
  }
  return ErrorCode;
}

bool libCM_test_EventFields_by_IndexArray(unsigned array_y, unsigned array_x, std::list <unsigned> const *EventFields, std::list <  std::list<uint16_t> > *index_array)
{
  if (EventFields==NULL) return true;
  if (index_array==NULL) return true;
  if (EventFields->size()<1) return true;
  if (index_array->size()<1) return true;
  if ((array_y*array_x)<=1) return true;
  bool RetVal = false;
  for (std::list<std::list<uint16_t> >::const_iterator it_1 = index_array->begin(); (it_1 != index_array->end()) && (!RetVal); it_1++)
  {
    unsigned value = 0;
    bool set = false;
           if (it_1->size()==1) // 1 dimension
    {
      set = true;
      value = *(it_1->begin());
    } else if (it_1->size()==2) // 2 dimensions
    {
      set = true;
      std::list<uint16_t> it_2 = (*it_1);
      unsigned y = *(it_2.begin());
      unsigned x = *(it_2.begin()++);
      value = (y * array_x) + x;
    }
    if (set)
    {
      for (std::list<unsigned>::const_iterator it_x = EventFields->begin(); (it_x != EventFields->end()) && (!RetVal); it_x++)
      {
        if ((*it_x) == value) RetVal = true;
      }
    }
  }
  return RetVal;
}

bool   libCM_reduce_EventFields_by_IndexArray(unsigned array_y, unsigned array_x, std::list <unsigned> *EventFields, std::list <  std::list<uint16_t> > *index_array) // retval = reduced any
{
  if (EventFields==NULL) return false;
  if (index_array==NULL) return false;
  if (EventFields->size()<1) return false;
  if (index_array->size()<1) return false;
  if ((array_y*array_x)<=1) return false;
  bool RetVal = false;

  for (std::list<unsigned>::iterator it_ev1 = EventFields->begin(); it_ev1 != EventFields->end();)
  {
    std::list<unsigned>::iterator it_ev = it_ev1++;
    bool found = false;
    for (std::list<std::list<uint16_t> >::const_iterator it_id = index_array->begin(); (it_id != index_array->end()) && (!found); it_id++)
    {
      unsigned value = 0;
      bool set = false;
           if (it_id->size()==1) // 1 dimension
      {
        set = true;
        value = *(it_id->begin());
      } else if (it_id->size()==2) // 2 dimensions
      {
        set = true;
        std::list<uint16_t> it_2 = (*it_id);
        unsigned y = *(it_2.begin());
        unsigned x = *(it_2.begin()++);
        value = (y * array_x) + x;
      }
      if (set)
      {
        if (value == *it_ev) found = true;
      }
    }
    if (!found) EventFields->erase(it_ev);
  }
  return RetVal;
}


// Roman tady puvodne mel permissions 644, to nevim proc?
void libCM_CreateDirectoryFromPath(const gchar *path)
{
  int LastSlashPos = -1;
  for (unsigned i=0;i<strlen(path);i++) if (path[i] == '/') LastSlashPos = i;
  if (LastSlashPos != -1)
  {
    gchar *testdir = (gchar *) g_new(gchar, LastSlashPos+2);
    strncpy(testdir, path, LastSlashPos+1);
    testdir[LastSlashPos+1] = 0;
    g_mkdir_with_parents(testdir, 0755);
    g_free(testdir);
  }
}

std::string libCM_GetProgramPath(void)
{
  char *path = (char *)malloc(PATH_MAX);
  std::string RetVal;
  if (path != NULL) 
  {
    int sz;
    if ((sz = readlink("/proc/self/exe", path, PATH_MAX)) != -1)
    {
      if ((sz>=0)&&(sz<PATH_MAX)) path[sz]=0;
      RetVal = std::string(path);
    }
    free(path);
  }
  return RetVal;
}


std::string libCM_GetLastNameFromPath(std::string path)
{
  std::string RetVal;
  char **item = g_strsplit (path.c_str(), "/", 0);
  if (item)
  {
    int items = 0;
    int i=0;
    while (item[i]) { i++; items++; }
    if (items>0) RetVal = std::string(item[items-1]);
    g_strfreev (item);
  }
  return RetVal;
}
