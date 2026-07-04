
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_base_functs.hh"
#include "cm_base_filters_set.hh"


CCmFiltersSet::CCmFiltersSet()
{
  smart = false;
  smart_clear();
  clear();
}
CCmFiltersSet::~CCmFiltersSet()
{
}

void CCmFiltersSet::set_smart(bool _smart)
{
  if (smart != _smart)
  {
    smart = _smart;
    smart_clear();
  }
}

void CCmFiltersSet::smart_clear(void)
{
  if (smart) smart_map.clear();
}

//
//void CCmFiltersSet::smart_undefine(std::string *varname)
//{
//  if (smart)
//  {
//    std::map<std::string, struct smart_item>::iterator it = smart_map.find(*varname);
//    if (it != smart_map.end())
//    {
//      smart_map.erase(it);
//    }
//  }
//}
//void CCmFiltersSet::smart_undefine(std::string &varname)
//{
//  smart_undefine(&varname);
//}
//void CCmFiltersSet::smart_undefine(char *varname)
//{
//  std::string varname2 = std::string(varname);
//  smart_undefine(&varname2);
//}





void CCmFiltersSet::clear(void)
{
  MyMapOnlyStars.clear();
  MyMapPatterns.clear();
  MyMapExact.clear();
  smart_clear();
  StarMaxCount = 0;
}
std::map <std::string, class CCmFilter> CCmFiltersSet::getMap(bool enableOnlyStars, bool enablePatterns,bool enableExact)
{
  std::map <std::string, class CCmFilter> RetMap;
  if (enableOnlyStars)
  {
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapOnlyStars.begin(); it != MyMapOnlyStars.end(); it++)
    {
      std::map <std::string, class CCmFilter>::iterator itout = RetMap.find(it->first);
      if (itout != RetMap.end())
      {
        itout->second.addIndexesSmart(it->second);
      } else {
        RetMap[it->first] = it->second;
      }
    }
  }
  if (enablePatterns)
  {
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
    {
      std::map <std::string, class CCmFilter>::iterator itout = RetMap.find(it->first);
      if (itout != RetMap.end())
      {
        itout->second.addIndexesSmart(it->second);
      } else {
        RetMap[it->first] = it->second;
      }
    }
  }
  if (enableExact)
  {
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapExact.begin(); it != MyMapExact.end(); it++)
    {
      std::map <std::string, class CCmFilter>::iterator itout = RetMap.find(it->first);
      if (itout != RetMap.end())
      {
        itout->second.addIndexesSmart(it->second);
      } else {
        RetMap[it->first] = it->second;
      }
    }
  }
  return RetMap;
}
std::map <std::string, class CCmFilter> CCmFiltersSet::getMapAll(void)
{
  return getMap(true, true, true);
}
std::map <std::string, class CCmFilter> CCmFiltersSet::getMapOnlyStars(void)
{
  return getMap(true, false, false);
}
std::map <std::string, class CCmFilter> CCmFiltersSet::getMapPatterns(void)
{
  return getMap(false, true, false);
}
std::map <std::string, class CCmFilter> CCmFiltersSet::getMapExact(void)
{
  return getMap(false, false, true);
}
bool CCmFiltersSet::add(class CCmFilter const *addf, bool DegradateOnlyStars, bool RemoveItBefore)
{
    bool changed = false;
    unsigned AddedStarCount = addf->getOnlyStarCount();

    // Pripad, kdy filter == "*"
    if ((DegradateOnlyStars) && (AddedStarCount == 1))
	return false; // OnlyStar==1 ... degradate => 0 ... no need add EMPTY
  
    class CCmFilter const *MyAdd = addf;
    std::map <std::string, class CCmFilter> *MyMap = &MyMapExact;
    CCmFilter ChangedFilter;
    if (AddedStarCount > 0)
    {
	// Filtr obsahuje pouze hvezdicky
	MyMap = &MyMapOnlyStars;
	if (DegradateOnlyStars)
	{
	    ChangedFilter = *addf;
	    std::string changename = ChangedFilter.getName();
	    if (changename.size()>0) changename = changename.substr(0,changename.size()-1);
	    ChangedFilter.setName(changename);
	    MyAdd = &ChangedFilter;
	    AddedStarCount = MyAdd->getOnlyStarCount();
	}
	if (StarMaxCount < AddedStarCount)
	    StarMaxCount = AddedStarCount;
    }
    else
    {
	if (addf->getFlagPattern())
	{
	    MyMap = &MyMapPatterns;
	}
	else
	{
	    MyMap = &MyMapExact;
	}
    }
    std::string name = MyAdd->getName();
    std::map <std::string, class CCmFilter>:: iterator it = MyMap->find(name);
    if (it != MyMap->end())
    {
	if (RemoveItBefore)
	{
	    changed = (!(it->second == (*MyAdd)));
	    it->second = *MyAdd;
	} else {
	    it->second.addIndexesSmart(*MyAdd, &changed);
	}
    } else {
	(*MyMap)[name] = *MyAdd;
	changed = true;
    }
    if (changed) smart_clear();
    return changed;
}
bool CCmFiltersSet::add(class CCmFilter const &addf, bool DegradateOnlyStars)
{
  return add(&addf, DegradateOnlyStars);
}
bool CCmFiltersSet::add(std::list <class CCmFilter> *list, bool DegradateOnlyStars)
{
  bool changed = false;
  for (std::list <class CCmFilter>::iterator it = list->begin(); it != list->end(); it++)
  {
    if (add(&(*it), DegradateOnlyStars)) changed = true;
  }
  return changed;
}
bool CCmFiltersSet::add(std::list <class CCmFilter> &list, bool DegradateOnlyStars)
{
  return add(&list, DegradateOnlyStars);
}
bool CCmFiltersSet::add(std::map <std::string, class CCmFilter> *map, bool DegradateOnlyStars)
{
  bool changed = false;
  for (std::map <std::string, class CCmFilter>::iterator it = map->begin(); it != map->end(); it++)
  {
    if (add(&(it->second), DegradateOnlyStars)) changed = true;
  }
  return changed;
}
bool CCmFiltersSet::add(std::map <std::string, class CCmFilter> &map, bool DegradateOnlyStars)
{
  return add(&map, DegradateOnlyStars);
}
bool CCmFiltersSet::add(class CCmFiltersSet *adds, bool DegradateOnlyStars)
{
  bool changed = false;
  std::map <std::string, class CCmFilter> map;
  map = adds->getMapAll();
  if (add(&map, DegradateOnlyStars)) changed = true;
//  map = adds->getMapOnlyStars();
//  if (add(&map, DegradateOnlyStars) changed = true; // s nejakou lepsi  logitou by to slo asi trosku urychlit
//  map = adds->getMapPatterns();
//  if (add(&map, DegradateOnlyStars) changed = true;
//  map = adds->getMapExact();
//  if (add(&map, DegradateOnlyStars) changed = true;
  return changed;
}
bool CCmFiltersSet::add(class CCmFiltersSet &adds, bool DegradateOnlyStars)
{
  return add(&adds, DegradateOnlyStars);
}
bool CCmFiltersSet::add(std::string *str, bool DegradateOnlyStars)
{
  CCmFilter NewFilter;
  NewFilter.fromString(*str);
  return add(&NewFilter, DegradateOnlyStars);
}
bool CCmFiltersSet::add(std::string &str, bool DegradateOnlyStars)
{
  return add(&str, DegradateOnlyStars);
}
bool CCmFiltersSet::add(const char *str, bool DegradateOnlyStars)
{
  std::string str2 = std::string(str);
  return add(&str2, DegradateOnlyStars);
}
bool CCmFiltersSet::change(class CCmFilter *addf, bool DegradateOnlyStars)
{
  return add(addf, DegradateOnlyStars, true);
}
bool CCmFiltersSet::change(class CCmFilter &addf, bool DegradateOnlyStars)
{
  return change(&addf, DegradateOnlyStars);
}
bool CCmFiltersSet::change(std::string *str, bool DegradateOnlyStars)
{
  CCmFilter NewFilter;
  NewFilter.fromString(*str);
  return change(&NewFilter, DegradateOnlyStars);
}
bool CCmFiltersSet::change(std::string &str, bool DegradateOnlyStars)
{
  return change(&str, DegradateOnlyStars);
}
bool CCmFiltersSet::change(const char *str, bool DegradateOnlyStars)
{
  std::string str2 = std::string(str);
  return change(&str2, DegradateOnlyStars);
}

bool CCmFiltersSet::erase(const std::string *str)
{
  bool changed = false;
  bool UpdateStarMaxCount = false;
  std::map <std::string, class CCmFilter> *MyMap = &MyMapExact;
  size_t star_count = std::count(str->begin(), str->end(), '*');
  if (star_count == str->size())
  {
    MyMap = &MyMapOnlyStars;
    if (star_count == StarMaxCount) UpdateStarMaxCount = true;
  } else {
    if (star_count>0) MyMap = &MyMapPatterns;
    if (str->find("?") != std::string::npos) MyMap = &MyMapPatterns;
    if (str->find("{") != std::string::npos) MyMap = &MyMapPatterns;
    if (str->find("}") != std::string::npos) MyMap = &MyMapPatterns;
  }
  std::map <std::string, class CCmFilter>::iterator it = MyMap->find(*str);
  if (it != MyMap->end())
  {
    MyMap->erase(it);
    changed = true;
  }
  if (UpdateStarMaxCount)
  {
    StarMaxCount = 0;
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapOnlyStars.begin(); it != MyMapOnlyStars.end(); it++)
    {
      unsigned StarCount = it->first.size();
      if (StarMaxCount < StarCount)
      {
        StarMaxCount = StarCount;
        changed = true;
      }
    }
  }
  if (changed) smart_clear();
  return changed;
}
bool CCmFiltersSet::erase(const std::string &str)
{
  return erase(&str);
}

bool CCmFiltersSet::erase(std::string *str)
{
  return erase((const std::string *)str);
}
bool CCmFiltersSet::erase(std::string &str)
{
  return erase((const std::string *)&str);
}
bool CCmFiltersSet::erase(const char *str)
{
  std::string str2 = std::string(str);
  return erase(&str2);
}
bool CCmFiltersSet::erase(std::list<std::string> *str)
{
  bool changed = false;
  for (std::list<std::string>::iterator it = str->begin(); it != str->end(); it++)
  {
    if (erase(&(*it))) changed = true;
  }
  return changed;
}
bool CCmFiltersSet::erase(std::list<std::string> &str)
{
  return erase(&str);
}
bool CCmFiltersSet::erase(std::map<std::string, class CCmVariable *> *map) // for cm_ext
{
  bool changed = false;
  for (std::map<std::string, class CCmVariable *>::iterator it = map->begin(); it != map->end(); it++)
  {
    if (erase(&it->first)) changed = true;
  }
  return changed;
}
bool CCmFiltersSet::erase(std::map<std::string, class CCmVariable *> &map) // for cm_ext
{
  return erase(&map);
}



bool CCmFiltersSet::test(class CCmFilter *filter, bool WithIndexes, std::string AddPrefix)
{
  std::map <std::string, class CCmFilter> *TestMap = &MyMapExact;
  if (filter->getFlagOnlyStar())
  {
    TestMap = &MyMapOnlyStars;
  } else if (filter->getFlagPattern())
  {
    TestMap = &MyMapPatterns;
  }
  std::string TestName = filter->getName();
  if (!AddPrefix.empty()) TestName = AddPrefix + TestName;
  std::map <std::string, class CCmFilter>::iterator it = TestMap->find(TestName);
  if (it != TestMap->end())
  {
    if (WithIndexes)
    {
      if (it->second.index_array == filter->index_array) return true; else return false;
    } else return true;
  }
  return false;
}
bool CCmFiltersSet::test(class CCmFilter &filter, bool WithIndexes, std::string AddPrefix)
{
  return test(&filter, WithIndexes, AddPrefix);
}




void CCmFiltersSet::gen_smart(std::string var_name, bool *result, std::list <  std::list<uint16_t> > *index_array)
{
  if (index_array!=NULL) index_array->clear();
  if (getFlagOnlyStar()) { *result = true; return; }

  std::map <std::string, class CCmFilter>::iterator it;
  it = MyMapExact.find(var_name);
  if (it != MyMapExact.end())
  {
//    if (it->second.test(var_name))
//    {
      *result = true;
      if (index_array!=NULL) *index_array = it->second.index_array; // simpy - it's first
//    }
  }
  for (it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
  {
    if (it->second.test(var_name))
    {
      *result = true;
      if (index_array!=NULL)
      {
        if ((index_array->size()>0) && (it->second.index_array.size()>0))
        {
        // MERGE ADD
//          bool changed = false;
//          libCM_IndexArray_add_smart(index_array, &it->second.index_array, &changed, &(it->second.getName()));
          libCM_IndexArray_add_smart(index_array, &it->second.index_array);
        } else index_array->clear();
      }
    }
  }
}
bool CCmFiltersSet::get_smart(std::string var_name, bool *result, std::list <  std::list<uint16_t> > *index_array)
{
// return true, if found REC
  std::map<std::string, struct smart_item>::iterator it = smart_map.find(var_name);
  if (it != smart_map.end())
  {
    *result = it->second.result;
    if (index_array != NULL) *index_array = it->second.index_array;
    return true;
  }
  return false;
}
void CCmFiltersSet::set_smart(std::string var_name, bool  result, std::list <  std::list<uint16_t> > *index_array)
{
  struct smart_item s;
  s.result = result;
  if (index_array != NULL) s.index_array = *index_array;
  smart_map[var_name] = s;
}

bool CCmFiltersSet::test(std::string TestName, std::string AddPrefix)
{
  if (smart)
  {
    std::string myTestName = TestName;
    bool RetVal = false;
    if (!AddPrefix.empty()) myTestName = AddPrefix + myTestName;
    if (get_smart(myTestName, &RetVal, NULL)) return RetVal;

    std::list <  std::list<uint16_t> > index_array;
    gen_smart(myTestName, &RetVal, &index_array);
    set_smart(myTestName,  RetVal, &index_array);
    return RetVal;
  } else {
    if (getFlagOnlyStar()) return true;
    std::string myTestName = TestName;
    if (!AddPrefix.empty()) myTestName = AddPrefix + myTestName;
    std::map <std::string, class CCmFilter>::const_iterator it;
    it = MyMapExact.find(myTestName);
    if (it != MyMapExact.end())
    {
      return true;
    }
    for (it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
    {
      if (it->second.test(TestName, AddPrefix)) return true;
    }
    return false;
  }
}

bool CCmFiltersSet::test(CCmVariable const *var, bool UseEventFields, std::string AddPrefix)
{
  if (smart)
  {
    std::string myTestName = var->getName();
    bool RetVal = false;
    if (!AddPrefix.empty()) myTestName = AddPrefix + myTestName;
    std::list <  std::list<uint16_t> > index_array;
    if (!get_smart(myTestName, &RetVal, &index_array))
    {
      gen_smart(myTestName, &RetVal, &index_array);
      set_smart(myTestName,  RetVal, &index_array);
    }
    if ((UseEventFields) && (index_array.size()>0) && (var->EventFields.size()))
    {
      RetVal = libCM_test_EventFields_by_IndexArray(var->getArrayY(), var->getArrayX(), &var->EventFields, &index_array);
    }
    return RetVal;
  } else {
    if (getFlagOnlyStar()) return true;
    std::string myTestName = var->getName();
    if (!AddPrefix.empty()) myTestName = AddPrefix + myTestName;

    std::map <std::string, class CCmFilter>::iterator it;
    it = MyMapExact.find(myTestName);
    if (it != MyMapExact.end())
    {
      if (it->second.test(var, UseEventFields, AddPrefix)) return true;
    }
    for (it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
    {
      if (it->second.test(var, UseEventFields, AddPrefix)) return true;
    }
    return false;
  }
}

bool CCmFiltersSet::test(CCmVariable const &var, bool UseEventFields, std::string AddPrefix)
{
  return test(&var, UseEventFields, AddPrefix);
}

bool CCmFiltersSet::test(CCmVariable const *var, std::list <  std::list<uint16_t> > *index_array, bool UseEventFields, std::string AddPrefix)
{
  if (smart)
  {
    std::string myTestName = var->getName();
    bool RetVal = false;
    if (!AddPrefix.empty()) myTestName = AddPrefix + myTestName;
    if (!get_smart(myTestName, &RetVal, index_array))
    {
      gen_smart(myTestName, &RetVal, index_array);
      set_smart(myTestName,  RetVal, index_array);
    }
    if ((UseEventFields) && (index_array!=NULL) &&  (index_array->size()>0) && (var->EventFields.size()))
    {
      RetVal = libCM_test_EventFields_by_IndexArray(var->getArrayY(), var->getArrayX(), &var->EventFields, index_array);
    }
    return RetVal;
  } else {
    std::list <  std::list<uint16_t> > *index_array_ptr = index_array;
    std::list <  std::list<uint16_t> > index_array_local;
    if (index_array == NULL) index_array_ptr = &index_array_local;
    index_array_ptr->clear();

    std::string myTestName = var->getName();
    bool RetVal = false;
    if (!AddPrefix.empty()) myTestName = AddPrefix + myTestName;
    gen_smart(myTestName, &RetVal, index_array_ptr);
    if ((UseEventFields) && (index_array_ptr->size()>0) && (var->EventFields.size()))
    {
      RetVal = libCM_test_EventFields_by_IndexArray(var->getArrayY(), var->getArrayX(), &var->EventFields, index_array_ptr);
    }
    return RetVal;
  }
}
bool CCmFiltersSet::test(CCmVariable const &var, std::list <  std::list<uint16_t> > &index_array, bool UseEventFields, std::string AddPrefix)
{
  return test(&var, &index_array, UseEventFields, AddPrefix);
}



    // compare cmFiltersSet / cmpMap std::map is OLD map, and myMap is NEW map
void CCmFiltersSet::diff( std::map <std::string, class CCmFilter> * myMap, std::map <std::string, class CCmFilter> *cmpMap,
                         class CCmFiltersSet *created, class CCmFiltersSet *deleted, class CCmFiltersSet *changed, bool CompareIndexes, bool ChangedToCreatedAndDeleted)
{
  if ((myMap == NULL) || (cmpMap == NULL)) return;
  for (std::map <std::string, class CCmFilter>::iterator itMy = myMap->begin(); itMy != myMap->end(); itMy++)
  {
    std::map <std::string, class CCmFilter>::iterator itCmp = cmpMap->find(itMy->first);
    if (itCmp == cmpMap->end())
    {
      if (created != NULL) created->add(itMy->second);
    } else {
      if (CompareIndexes)
      {
        if (itCmp->second.index_array != itMy->second.index_array)
        {
          if (ChangedToCreatedAndDeleted)
          {
            if (created != NULL) created->add(itMy->second);
            if (deleted != NULL) deleted->add(itCmp->second);
          } else {
            if (changed != NULL) changed->add(itMy->second);
          }
        }
      }
    }
  }
  if (deleted != NULL)
  {
    for (std::map <std::string, class CCmFilter>::iterator itCmp = cmpMap->begin(); itCmp != cmpMap->end(); itCmp++)
    {
      std::map <std::string, class CCmFilter>::iterator itMy = myMap->find(itCmp->first);
      if (itMy == myMap->end())
      {
        deleted->add(itCmp->second);
      }
    }
  }
}

    // compare cmFiltersSet / cmpMap std::map is OLD map, and myMap is NEW map
void CCmFiltersSet::diff(class CCmFiltersSet *compare,
                         class CCmFiltersSet *created,
                         class CCmFiltersSet *deleted,
                         class CCmFiltersSet *changed,
                         bool CompareIndexes,
                         bool ChangedToCreatedAndDeleted)
{
  if (created != NULL) created->clear();
  if (deleted != NULL) deleted->clear();
  if (changed != NULL) changed->clear();
  diff(&MyMapOnlyStars, compare->getMapOnlyStarsPTR(), created, deleted, changed, CompareIndexes, ChangedToCreatedAndDeleted);
  diff(&MyMapPatterns , compare->getMapPatternsPTR() , created, deleted, changed, CompareIndexes, ChangedToCreatedAndDeleted);
  diff(&MyMapExact    , compare->getMapExactPTR()    , created, deleted, changed, CompareIndexes, ChangedToCreatedAndDeleted);
}
void CCmFiltersSet::diff(class CCmFiltersSet &compare, class CCmFiltersSet *created, class CCmFiltersSet *deleted, class CCmFiltersSet *changed, bool CompareIndexes, bool ChangedToCreatedAndDeleted)
{
  diff(&compare, created, deleted, changed, CompareIndexes, ChangedToCreatedAndDeleted);
}

std::string CCmFiltersSet::getNames(std::string delimiter, bool enableOnlyStars, bool enablePatterns, bool enableExact)
{
  std::string RetVal;
  if (enableOnlyStars)
  {
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapOnlyStars.begin(); it != MyMapOnlyStars.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.getName();
    }
  }
  if (enablePatterns)
  {
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.getName();
    }
  }
  if (enableExact)
  {
    for (std::map <std::string, class CCmFilter>::iterator it = MyMapExact.begin(); it != MyMapExact.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.getName();
    }
  }
  return RetVal;
}
std::string CCmFiltersSet::toString(std::string delimiter, bool enableOnlyStars, bool enablePatterns, bool enableExact) const
{
  std::string RetVal;
  if (enableOnlyStars)
  {
    for (std::map <std::string, class CCmFilter>::const_iterator it = MyMapOnlyStars.begin(); it != MyMapOnlyStars.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.toString();
    }
  }
  if (enablePatterns)
  {
    for (std::map <std::string, class CCmFilter>::const_iterator it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.toString();
    }
  }
  if (enableExact)
  {
    for (std::map <std::string, class CCmFilter>::const_iterator it = MyMapExact.begin(); it != MyMapExact.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.toString();
    }
  }
  return RetVal;
}
std::string CCmFiltersSet::fullPrint(std::string delimiter, bool enableOnlyStars, bool enablePatterns, bool enableExact) const
{
  std::string RetVal;
  if (smart)
  {
    char WStr[100];
    sprintf(WStr,"%d",(int)smart_map.size());
    RetVal += std::string("smart_map_size=") + std::string(WStr);
  }
  if (enableOnlyStars)
  {
    for (std::map <std::string, class CCmFilter>::const_iterator it = MyMapOnlyStars.begin(); it != MyMapOnlyStars.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.fullPrint();
    }
  }
  if (enablePatterns)
  {
    for (std::map <std::string, class CCmFilter>::const_iterator it = MyMapPatterns.begin(); it != MyMapPatterns.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.fullPrint();
    }
  }
  if (enableExact)
  {
    for (std::map <std::string, class CCmFilter>::const_iterator it = MyMapExact.begin(); it != MyMapExact.end(); it++)
    {
      if (!RetVal.empty()) RetVal += delimiter;
      RetVal += it->second.fullPrint();
    }
  }
  return RetVal;
}

