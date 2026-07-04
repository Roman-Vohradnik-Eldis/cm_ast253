
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_base_functs.hh"
#include "cm_base_filter.hh"




CCmFilter::CCmFilter()
{
  flags = 0;
  name = std::string("");
}

bool CCmFilter::operator == (const CCmFilter &rhs)
{
  return (name == rhs.name) && (index_array == rhs.index_array);
}



void CCmFilter::setName(std::string _name)
{
  name = _name;
  flags = 0;
  bool flagPattern = false;
  bool flagPatternReplace = false;

  size_t star_count = std::count(name.begin(), name.end(), '*');
  if (star_count == name.size())
  {
    uint16_t sc = MIN(star_count,255);
    flags |= (1<<0) | ((sc & 0xff)<<8);
  } else {
    if (star_count>0) flagPattern = true;
    if (_name.find("?") != std::string::npos) flagPattern = true;
    if (_name.find("{") != std::string::npos) flagPatternReplace = true;
    if (_name.find("}") != std::string::npos) flagPatternReplace = true;
    if (flagPatternReplace)
    {
      cm_std_string_replace(name,"{","[");
      cm_std_string_replace(name,"}","]");
    }
    if (flagPattern) flags |= (1<<0);
    if (flagPatternReplace) flags |= (1<<0) | (1<<1);
  }
  // pripadne tady pak nahradit "{"->"[" .... "}"->"]"  , pak by se ale taky muselo zase upravit zakodovani toString()
}

std::string CCmFilter::getName(void) const
{
  std::string RetVal = name;
  if (flags & (1<<1))
  {
    cm_std_string_replace(RetVal,"[","{");
    cm_std_string_replace(RetVal,"]","}");
  }
  return RetVal;
}

std::string CCmFilter::getNameRAW(void)
{
  return name;
}

std::string CCmFilter::index_array_toString(void) const
{
  std::string RetVal = std::string("");

  bool first_i1 = true;
  for (std::list <  std::list<uint16_t> >::const_iterator it_i1 = index_array.begin(); it_i1 != index_array.end(); it_i1++)
  {
    if (!first_i1) RetVal += std::string("/");
    bool first_i2 = true;
    for (std::list<uint16_t>::const_iterator it_i2 = it_i1->begin(); it_i2 != it_i1->end(); it_i2++)
    {
      if (!first_i2) RetVal += std::string(",");
      char val[20];
      sprintf(val,"%d",(*it_i2));
      RetVal += std::string(val);
      first_i2 = false;
    }
    first_i1 = false;
  }

  return RetVal;
}


std::string CCmFilter::toString(void) const
{
  std::string RetVal = name;
  if (flags & (1<<1))
  {
    cm_std_string_replace(RetVal,"[","{");
    cm_std_string_replace(RetVal,"]","}");
  }

  std::string IdxStr = index_array_toString();
  if (IdxStr.size()>0)
  {
    RetVal += std::string("[") + IdxStr + std::string("]") ;
  }
  return RetVal;
}

std::string CCmFilter::fullPrint(void) const
{
  return toString();
}



void CCmFilter::fromString(std::string inp)
{
// debug_printf("FROM_STRING \"%s\"",inp.c_str());
  index_array.clear();
  char **item = g_strsplit (inp.c_str(), "[", 0);
  if (item && item[0])
  {
    setName(std::string(item[0]));
//   debug_printf("  - name=\"%s\"",name.c_str());
    if (item[1])
    {
      char **item_i1 = g_strsplit (item[1], "]", 0);
      if (item_i1 && item_i1[0])
      {
        char **item_i2 = g_strsplit (item_i1[0], "/", 0);
        if (item_i2 && item_i2[0])
        {
          unsigned i2 = 0;
          while (item_i2[i2])
          {
//           debug_printf ("   - ");
            char **item_i3 = g_strsplit (item_i2[i2], ",", 0);
            if (item_i3 && item_i3[0])
            {
              std::list<uint16_t> w_list;
              w_list.clear();
              unsigned i3 = 0;
              while (item_i3[i3])
              {
                uint16_t idx = cm_string_get_value_int(item_i3[i3]);
                w_list.push_back(idx);
//               debug_printf("%",idx);
                i3++;
              }
              index_array.push_back(w_list);
              g_strfreev (item_i3);
            }
            i2++;
//           debug_printf ("\n");
          }
          g_strfreev (item_i2);
        }
        g_strfreev (item_i1);
      }
    }
    g_strfreev (item);
  }
}

unsigned CCmFilter::addIndex  (             std::list<uint16_t>   add_index_item , bool *changed)
{
//  unsigned ErrorCode = 0;
//  if (changed != NULL) *changed = false;
//
//  unsigned dimensions = add_index_item.size();
//  if (index_array.size()>0)
//  {
//    if (index_array.begin()->size() != dimensions)
//    {
//      cms_ns_if_print("libcm",1, "\"%s\" - cannot add index array with different dimensions %d != %d",name.c_str(),index_array.begin()->size(),dimensions);
//      return 1;
//    }
//  }
//  if ((dimensions>0)&&(dimensions<=2))
//  {
//    bool found_same = false;
//    for (std::list <  std::list<uint16_t> >::iterator it_dst_idx = index_array.begin(); it_dst_idx != index_array.end(); it_dst_idx++)
//    {
//      if ((*it_dst_idx) == (add_index_item)) found_same = true; // skutecne funguje takoveto porovnani???
//    }
//    if (!found_same)
//    {
//      index_array.push_back( add_index_item );
//      if (changed != NULL) *changed = true;
//    }
//  } else {
//    cms_ns_if_print("libcm",1, "\"%s\" - cannot make array with %d dimensions",name.c_str(),dimensions);
//    ErrorCode = 2;
//  }
//  return ErrorCode;

  return libCM_IndexArray_add_smart(&index_array, &add_index_item, changed, &name);


}

//unsigned CCmFilter::addIndexes(std::list <  std::list<uint16_t> > add_index_array, bool *changed)
//{
//  unsigned ErrorCode = 0;
//  if (changed != NULL) *changed = false;
//  for (std::list <  std::list<uint16_t> >::iterator it_src_idx = add_index_array.begin(); (it_src_idx != add_index_array.end()) && (!ErrorCode); it_src_idx++)
//  {
//    bool local_changed = false;
//    unsigned Ecode = addIndex  (*it_src_idx, &local_changed);
//    if (Ecode) ErrorCode = Ecode;
//    if ((local_changed)&&(changed != NULL)) *changed = true;
//  }
//  return ErrorCode;
//}

unsigned CCmFilter::addIndexesSmart(std::list <  std::list<uint16_t> > add_index_array, bool *changed)
{
//  unsigned ErrorCode = 0;
//  if (changed != NULL) *changed = false;
//  if ((index_array.size()>0)&&(add_index_array.size()>0))
//  {
//    for (std::list <  std::list<uint16_t> >::iterator it_src_idx = add_index_array.begin(); (it_src_idx != add_index_array.end()) && (!ErrorCode); it_src_idx++)
//    {
//      bool local_changed = false;
//      unsigned Ecode = addIndex  (*it_src_idx, &local_changed);
//      if (Ecode) ErrorCode = Ecode;
//      if ((local_changed)&&(changed != NULL)) *changed = true;
//    }
//  } else {
//    if ((index_array.size()>0)&&(changed != NULL)) *changed = true;
//    index_array.clear();
//  }
//  return ErrorCode;
  return libCM_IndexArray_add_smart(&index_array, &add_index_array, changed, &name);
}

unsigned CCmFilter::addIndexesSmart(const CCmFilter &rhs, bool *changed)
{
  return addIndexesSmart(rhs.index_array, changed);
}

bool CCmFilter::test(std::string TestName, std::string AddPrefix) const
{
  if (flags & (0xff<<8)) return true; // onlystars
  if (flags & (1<<0)) // pattern?
  {
    if (AddPrefix.empty())
    {
      return libCM_isNameMatching(name, TestName);
    } else {
      return libCM_isNameMatching(name, (AddPrefix+TestName));
    }
  } else {
    if (AddPrefix.empty())
    {
      return (TestName == name);
    } else {
      return ((AddPrefix+TestName) == name);
    }
  }
}


bool CCmFilter::test(CCmVariable const *var, bool UseEventFields, std::string AddPrefix)
{
    if (test(var->getName(), AddPrefix))
    {
	if ((UseEventFields)&&(var->isArray()) && (index_array.size()>0) && (var->EventFields.size()>0))
	{
	    bool RetVal = false;
	    unsigned ArrayX = var->getArrayX();
	    for (std::list<std::list<uint16_t> >::iterator it_1 = index_array.begin(); (it_1 != index_array.end()) && (!RetVal); it_1++)
	    {
		unsigned value = 0; // absolutní index.
		bool set = false;
		if (it_1->size()==1) // 1 dimension
		{
		    set = true;
		    value = *(it_1->begin());
		}
		else if (it_1->size()==2) // 2 dimensions
		{
		    set = true;
		    std::list<uint16_t> it_2 = (*it_1);
		    unsigned y = *(it_2.begin());
		    unsigned x = *(it_2.begin()++);
		    value = (y * ArrayX) + x;
		}
		if (set)
		{
		    for (std::list<unsigned>::const_iterator it_x = var->EventFields.begin(); (it_x != var->EventFields.end()) && (!RetVal); it_x++)
		    {
			if ((*it_x) == value) RetVal = true;
		    }
		}
	    }
	    return RetVal;
	}
	return true;
    }
    return false;
}

bool CCmFilter::test(CCmVariable const &var, bool UseEventFields, std::string AddPrefix)
{
  return test(&var, UseEventFields, AddPrefix);
}

