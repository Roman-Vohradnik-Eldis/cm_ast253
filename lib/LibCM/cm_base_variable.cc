#include "cm_header_internal.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
#include "cm_base_functs.hh"
#include "cm_base_variable.hh"
#include "cm_utils.hh"
#include "cm_debug.hh"

#include <fstream>
#include <functional>
#include <sstream>

// # name         = priority, max_update_time, flags, type, array_y:array_x=default_value,                               ,value(s)     # bool
// # name         = priority, max_update_time, flags, type, array_y:array_x=default_value, min_value, max_value, min_step,value(s)     # float, int
// # name         = priority, max_update_time, flags, type, array_y:array_x=default_value, enum_values                   ,value(s)     # enum(int)
// # name         = priority, max_update_time, flags, type, array_y:array_x=default_value, string_max_size               ,value(s)     # string
// # priority: 0-99, 0=highest, 99=lowest.  1=don't respect max_update_time and send immediately, but respect baud_speed, 0=same as 1, but don't respect baud_speed
// # flags : external | writable | save_file 
// # max_update_time [sec] ... if variable was sent now, next send is blocked until this seted time
// # enum values example : 0="M1":1="M2":3="M3/A":6="MC":23="empty"
// #  values :  for single .. one value, for array ... value1, value2, .. valueN
// #    array default pointer y,x = 0,0 on start changing, but can be moved : value[0,0], value[0,1], 3:3=value[3,3], value[3,4], ...
// #    for set more same values (FILL) can be used also y_'y_count':x'x_count' : value[0,0], ....  7_3:3_2=FILLVALUE[7..9,3..4], value[9,5]

void CCmVariable::check_consistency()
{
    if (type == 'i' || type == 'b')
        assert(!i.empty()); // je tam default value
    // if (type == 'b')
    //     for (auto x : i)
    //         assert(x == 0L || x == 1L);
}

CCmVariable::CCmVariable()
    : type('?')
{
    ConsistencyCheck cch(this);
    SendBlockUntil = 0.0; MaxUpdateFreq = 0.0; flags=0;
    setArrayYX(1, 1);
    setLimitsS(std::string().max_size());
    setLimitsF(DBL_MIN, DBL_MAX, DBL_EPSILON);
    // setFlagSigned(true);
    setLimitsI(INT64_MIN, INT64_MAX, 1L);
}

// template <typename T>
// size_t sum_capacity(T const &c)
// {
//     size_t result = 0;
//     for (auto &x : c)
//         result += x.capacity() + sizeof(x);
//     return result;
// }

char CCmVariable::getType() const
{
  return type;
}

void CCmVariable::setType(char n)
{
    ConsistencyCheck cch(this);
    type=n;
    unsigned arr_size = array_y * array_x;
    if ((type=='i')||(type=='e')||(type=='b')) i.resize(arr_size); else i.resize(0);
    if (type=='f') f.resize(arr_size); else f.resize(0);
    if (type=='s') s.resize(arr_size); else s.resize(0);
}
std::string CCmVariable::getName() const
{
  return name;
}
void CCmVariable::setName(std::string _name)
{
    ConsistencyCheck cch(this);
    if (!libCM_is_valid_variable_name(_name))
    {
        abort();
        // throw CmInvalidVariableName(std::string("Invalid variable name: `'") + name);
    }
    name = _name;
}
bool CCmVariable::isArray() const
{
  return ((array_y > 1) || (array_x > 1)); 
}
unsigned CCmVariable::getArrayY() const
{
  return array_y; 
}
unsigned CCmVariable::getArrayX() const
{
  return array_x;
}
void CCmVariable::setArrayYX(unsigned Y, unsigned X)
{
    ConsistencyCheck cch(this);
  array_y = Y;
  array_x = X;
  if (array_y < 1) array_y = 1;
  if (array_x < 1) array_x = 1;
  unsigned arr_size = array_y * array_x;
  if ((type=='i')||(type=='e')||(type=='b')) i.resize(arr_size); else i.resize(0);
  if (type=='f') f.resize(arr_size); else f.resize(0);
  if (type=='s') s.resize(arr_size); else s.resize(0);
}

uint8_t CCmVariable::getPriority() const
{
  return priority;
}
void CCmVariable::setPriority(uint8_t newPriority)
{
    ConsistencyCheck cch(this);
  priority = newPriority;
}

bool CCmVariable::compareType(const CCmVariable &rhs) const
{
  return ((type==rhs.type) && (array_y==rhs.array_y) && (array_x==rhs.array_x));
}

bool CCmVariable::compareTypeAndLimits(const CCmVariable &rhs) const
{
  if ((type==rhs.type) && (array_y==rhs.array_y) && (array_x==rhs.array_x))
  {
      if ((type=='i')) return (i_min == rhs.i_min) && (i_max == rhs.i_max) && (i_step == rhs.i_step);
    if (type=='f') return (f_min == rhs.f_min) && (f_max == rhs.f_max) && (f_step == rhs.f_step);
    if (type=='s') return (s_maxsize == rhs.s_maxsize);
    if (type=='e') return (enum_values == rhs.enum_values) && (enum_sorting == rhs.enum_sorting);
//    if (type=='e') return (enum_values == rhs.enum_values);
    return true; // type 'e' and 'b'
  }
  return false;
}

void CCmVariable::copy_limits(const CCmVariable &rhs)
{
    ConsistencyCheck cch(this);
  if ((type==rhs.type) && (array_y==rhs.array_y) && (array_x==rhs.array_x))
  {
    i_min = rhs.i_min;
    i_max = rhs.i_max;
    i_step = rhs.i_step;
    f_min = rhs.f_min;
    f_max = rhs.f_max;
    f_step = rhs.f_step;
    s_maxsize = rhs.s_maxsize;
    enum_values = rhs.enum_values;
    enum_strings = rhs.enum_strings;
    enum_sorting = rhs.enum_sorting;
  }
}


bool CCmVariable::haveFlagWritable() const
{ return (((flags >> CM_VARIABLE_FLAG_WRITABLE) & 1) ? true : false); }
bool CCmVariable::haveFlagExternal() const
{ return (((flags >> CM_VARIABLE_FLAG_EXTERNAL) & 1) ? true : false); }
bool CCmVariable::haveFlagSaveFile() const
{ return (((flags >> CM_VARIABLE_FLAG_SAVEFILE) & 1) ? true : false); }
// bool CCmVariable::haveFlagSigned() const
// { return (((flags >> CM_VARIABLE_FLAG_SIGNED) & 1) ? true : false); }
bool CCmVariable::haveFlagVirtual() const
{ return (((flags >> CM_VARIABLE_FLAG_VIRTUAL) & 1) ? true : false); }
bool CCmVariable::haveFlagPriorityChanged() const
{ return (((flags >> CM_VARIABLE_FLAG_PRIORITY_CHANGED) & 1) ? true : false); }
bool CCmVariable::haveFlagForeign() const
{ return (((flags >> CM_VARIABLE_FLAG_FOREIGN) & 1) ? true : false); }
bool CCmVariable::haveFlagOwned() const
{ return (((flags >> CM_VARIABLE_FLAG_OWNED) & 1) ? true : false); }



void CCmVariable::setFlagPriorityChanged(bool newVal)
{
    ConsistencyCheck cch(this);
    if (newVal)
	flags |= (1 << CM_VARIABLE_FLAG_PRIORITY_CHANGED);
    else
	flags &= ~(uint8_t)(1 << CM_VARIABLE_FLAG_PRIORITY_CHANGED);
}
void CCmVariable::setFlagVirtual(bool newVal)
{
    ConsistencyCheck cch(this);
    if (newVal)
	flags |= (1 << CM_VARIABLE_FLAG_VIRTUAL);
    else
	flags &= ~(uint8_t)(1 << CM_VARIABLE_FLAG_VIRTUAL);
}

void CCmVariable::setFlagForeign(bool newVal)
{
    ConsistencyCheck cch(this);
    if (newVal)
	flags |= (1 << CM_VARIABLE_FLAG_FOREIGN);
    else
	flags &= ~(uint8_t)(1 << CM_VARIABLE_FLAG_FOREIGN);
}

void CCmVariable::setFlagOwned(bool newVal)
{
    ConsistencyCheck cch(this);
    if (newVal)
	flags |= (1 << CM_VARIABLE_FLAG_OWNED);
    else
	flags &= ~(uint8_t)(1 << CM_VARIABLE_FLAG_OWNED);
}

void CCmVariable::getLimitsF(double &_f_min, double &_f_max, double &_f_step) const  { _f_min = f_min; _f_max = f_max; _f_step = f_step; }
void CCmVariable::getLimitsI(libcm_integer &_i_min, libcm_integer &_i_max, libcm_integer &_i_step) const  { _i_min = i_min; _i_max = i_max; _i_step = i_step; }
void CCmVariable::getLimitsS(unsigned &_s_maxsize) const{ _s_maxsize = s_maxsize; }
void CCmVariable::setLimitsS(unsigned _s_maxsize)
{
    ConsistencyCheck cch(this);
    s_maxsize = _s_maxsize;
}
void CCmVariable::setLimitsI(libcm_integer _i_min, libcm_integer _i_max, libcm_integer _i_step)
{
    ConsistencyCheck cch(this);
    i_min = _i_min; i_max = _i_max; i_step = _i_step;
}
void CCmVariable::setLimitsF(double   _f_min, double   _f_max, double   _f_step)
{
    ConsistencyCheck cch(this);
    f_min = _f_min; f_max = _f_max; f_step = _f_step; recalculate_max_decimals();
}
std::list< int64_t> CCmVariable::getEnumSorting() const { return enum_sorting; }
std::map< int64_t, std::string> CCmVariable::getEnumValues() const { return enum_values; }
std::map< std::string, int64_t> CCmVariable::getEnumStrings() const { return enum_strings; }
void CCmVariable::clearEventFields(void)
{
    ConsistencyCheck cch(this);
    EventFields.clear();
}
void CCmVariable::setModifTime(double Time)
{
    ConsistencyCheck cch(this);
    modif_time = Time;
}
double CCmVariable::getModifTime() const { return modif_time; }


int calculate_max_decimals(double f_step)
{
    std::string wstr = cm_std_string_print_short_float(f_step);;
    if (wstr == "0")
	return 10;
    
    int decimals = 0;
    bool was_dot = false;
    for (unsigned i=0; i<wstr.size(); i++)
    {
	if (was_dot)
	{
	    assert(wstr[i] >= '0' && wstr[i] <= '9');
	    ++decimals;
	}
	else
	{
	    if (wstr[i] == '.')
		was_dot = true;	    
	}
    }
    if (!was_dot)
    {
	return 0; // f_step je cele cislo
    }
    else
    {
	assert(decimals > 0); // za teckou neco musi byt, jinak by tam nebyla.
	return decimals;
    }
}

void CCmVariable::recalculate_max_decimals(void)
{
    ConsistencyCheck cch(this);
    max_decimals = calculate_max_decimals(f_step);
}

int CCmVariable::getMaxDecimals() const { return max_decimals; }

bool CCmVariable::compare_idx(const CCmVariable &rhs,  unsigned idx) const
{
    bool RetVal = false;
    if ((type == 'b') || (type == 'i') || (type == 'e'))
    {
	if (i[idx] == rhs.i[idx])
	{
	    RetVal = true;
	}
    }
    else if (type == 's')
    {
	if (s[idx] == rhs.s[idx])
	{
	    RetVal = true;
	}
    }
    else if (type == 'f')
    {
	if (f[idx] == rhs.f[idx])
	{
	    RetVal = true;
	}
    }
    return RetVal;
}

void CCmVariable::copy_one_index(const CCmVariable &rhs, unsigned ptr)
{
    ConsistencyCheck cch(this);
  if ((type == 'b') || (type == 'i') || (type == 'e'))
  {
    i[ptr] = rhs.i[ptr];
  }
  if (type == 's')
  {
    s[ptr] = rhs.s[ptr];
  }
  if (type == 'f')
  {
    f[ptr] = rhs.f[ptr];
  }
}

void CCmVariable::copy_indexes(const CCmVariable &rhs, std::list <unsigned> Fields)
{
    ConsistencyCheck cch(this);
  unsigned field_max = array_y * array_x;
  if (Fields.size()>0)
  {
    for (std::list <unsigned>::iterator it = Fields.begin(); it != Fields.end(); it++)
//    for (std::list <unsigned>::iterator it = EventFields.begin(); it != EventFields.end(); it++)
    {
      unsigned ptr = *it;
      if (ptr < field_max)
      {
        copy_one_index(rhs, ptr);
      }
    }
  } else {
    for (unsigned ptr=0; ptr<field_max; ptr++)
    {
      copy_one_index(rhs, ptr);
    }
  }
 
}

bool CCmVariable::diffFields(const CCmVariable &rhs)
{
    ConsistencyCheck cch(this);
    bool RetVal = false;
    if (type == rhs.type)
    {
        if ((array_x == rhs.array_x) && (array_y == rhs.array_y))
        {

            if ((type == 'b') || (type == 'i') || (type == 'e'))
            {
                if ((i.size() != rhs.i.size()) || (i.size()<(array_y * array_x)))
                    return false;
            }
            if (type == 's')
            {
                if ((s.size() != rhs.s.size()) || (s.size()<(array_y * array_x)))
                    return false;
            }
            if (type == 'f')
            {
                if ((f.size() != rhs.f.size()) || (f.size()<(array_y * array_x)))
                    return false;
            }

            if (isArray())
            {
                std::list <unsigned> new_EventFields;
                unsigned field_max = array_y * array_x;
                if (EventFields.size()>0)
                {
                    for (std::list <unsigned>::iterator it = EventFields.begin(); it != EventFields.end(); it++)
                    {
                        unsigned ptr = *it;
                        if (ptr < field_max)
                        {
                            // porovnej ptr, nastav retval a zarad do new_EventFields
                            if (!compare_idx(rhs,ptr))
                            {
                                RetVal = true;
                                new_EventFields.push_back(ptr);
                            }
                        }
                    }
                } else
                {
                    for (unsigned ptr=0; ptr<field_max; ptr++)
                    {
                        // porovnej ptr, nastav retval a zarad do new_EventFields
                        if (!compare_idx(rhs,ptr))
                        {
                            RetVal = true;
                            new_EventFields.push_back(ptr);
                        }
                    }
                }
                EventFields = new_EventFields;
            } else {
                EventFields.clear();
                if (!compare_idx(rhs,0)) RetVal = true;
            }
        }
    }
    return RetVal;
}

bool CCmVariable::diffAllFields(const CCmVariable &rhs)
{
    ConsistencyCheck cch(this);
  EventFields.clear();
  return diffFields(rhs);
}


bool CCmVariable::compare(const CCmVariable &rhs) const
{
    bool RetVal = false;
    if (compareTypeAndLimits(rhs))
    {
        if ((type == 'b') || (type == 'i') || (type == 'e'))
        {
	    if (i.size() == rhs.i.size())
	    {
		if (i == rhs.i)
		{
		    RetVal = true;
		}
	    }
        }
        if (type == 's')
        {
	    if (s.size() == rhs.s.size())
	    {
		if (s == rhs.s)
		{
		    RetVal = true;
		}
	    }
        }
        if (type == 'f')
        {
	    if (f.size() == rhs.f.size())
	    {
		if (f == rhs.f)
		{
		    RetVal = true;
		}
	    }
        }
    }
    return RetVal;
}


bool CCmVariable::operator==(const CCmVariable &rhs) const
{
  return compare(rhs);
}

bool CCmVariable::operator!=(const CCmVariable &rhs) const
{
  return !compare(rhs);
}

// Serializuje hodnotu na absolutni pozici
std::string CCmVariable::getValue(unsigned idx) const
{
    std::string retstr = std::string("");
    if (type=='b')
    {
	assert(i[idx] == 0L || i[idx] == 1L);
	if (i[idx] != 0L)
	    retstr = std::string("Y");
	else
	    retstr = std::string("N");
    }
    else if ((type=='i')||(type=='e'))
    {
	char s_val[100];
	// if ((flags >> CM_VARIABLE_FLAG_SIGNED) & 1)
	// {
	sprintf(s_val,"%" PRId64 "",i[idx]);
	// } else {
	//     sprintf(s_val,"%" PRIu64 "",i[idx]);
	// }
	retstr = std::string(s_val);
    }
    else if (type=='f')
    {
	retstr = cm_std_string_print_short_float(f[idx], max_decimals);
	std::stringstream ss;
	ss << f[idx];
	cms_ns_if_print("libcm", 4, "Print float %s: %s, retstr=%s, max_decimals=%d", name.c_str(), ss.str().c_str(), retstr.c_str(), max_decimals);
    }
    else if (type=='s')
    {
	retstr = "\"" + escape_special_chars(s[idx]) + "\"";
	// gchar * s_val = cm_string_escape_string(s[idx].c_str(), "#\n\"\r\t\\", true);
	// if (s_val)
	// {
	//     retstr = std::string(s_val);
	//     g_free(s_val);
	// }
    }
    else
    {
	assert(false);
    }

    return retstr;
}

// inp: hodnota float/int/string/bool reprezentovana jako string
// idx: od ktereho indexu
// count: nastavujeme idx+0,...,idx+(count-1)
// ProtectedIndexes: vyjma techto vyjmenovanych indexu
bool CCmVariable::setValues(std::string inp, unsigned idx, unsigned count, std::list <unsigned> *ProtectedIndexes)
{
    ConsistencyCheck cch(this);
    bool changed = false;
    libcm_integer iv = 0L;
    double fv = 0.0;
    std::string sv = std::string("");

    if (type=='b')
    {
	iv = cm_string_get_value_int64(inp.c_str());
	if (iv != 0L && iv != 1L)
	    cms_ns_if_print("libcm", 1, "Changing Bool variable %s with setValues %s: Expected 0/1, got %" PRId64". Converted to TRUE.", name.c_str(), inp.c_str(), iv);
	if (iv != 0L)
	    iv = 1;
    }
    else if (type=='i')
    {
	// if ((flags >> CM_VARIABLE_FLAG_SIGNED) & 1) // flagSigned
	// {
	//     iv = (uint64_t)cm_string_get_value_int64(inp.c_str());
	//     if (iv < i_min) iv = i_min;
	//     if (iv > i_max) iv = i_max;
	// }
	// else
	{
	    iv = cm_string_get_value_int64(inp.c_str());
	    if (iv < i_min) iv = i_min;
	    if (iv > i_max) iv = i_max;
	}
    }
    else if (type=='f')
    {
	fv = cm_string_get_value_double(inp.c_str());
	if (fv < f_min) fv = f_min;
	if (fv > f_max) fv = f_max;
    }
    else if (type=='s')
    {
	gchar * sv_c = cm_string_interpret_first_string(inp.c_str());
	if (sv_c)
	{
	    sv = std::string(sv_c);
	    g_free(sv_c);
	}
    }
    else if (type=='e')
    {
	gchar * sv_c = cm_string_interpret_first_string(inp.c_str());
	iv = 0;
	bool found = false;
	if (sv_c)
	{
	    std::string sv = std::string(sv_c);
	    std::map <std::string, int64_t>::iterator dit = enum_strings.find(sv);
	    if (dit != enum_strings.end())
	    {
		iv = dit->second;
		found = true;
	    }
	    g_free(sv_c);
	}
	if (!found)
	{
	    iv = cm_string_get_value_int64(inp.c_str());
	    std::map <int64_t, std::string>::iterator vit = enum_values.find(iv); // existuje?
	    if (vit == enum_values.end())
	    {
		if (enum_strings.begin() != enum_strings.end())
		    iv = enum_strings.begin()->second; // dej prvni cislo volbu
	    }
	}
    }

    for (unsigned x=0;x<count;x++)
    {
	bool use = true;
	if (ProtectedIndexes != NULL)
	{
	    for ( std::list <unsigned>::iterator it = ProtectedIndexes->begin(); it != ProtectedIndexes->end(); it++)
	    {
		if ( (*it) == (idx+x) ) use = false;
	    }
	}
	if (use)
	{
	    if ((type=='i') || (type=='b') || (type=='e'))
	    {
		if (i[idx+x] != iv) changed = true;
		i[idx+x] = iv;
		if (type == 'b')
		    assert(i[idx+x] == 0L || i[idx+x] == 1L);
	    } else if (type=='f')
	    {
		if (f[idx+x] != fv) changed = true;
		f[idx+x] = fv;
	    } else if (type=='s')
	    {
		if (s[idx+x] != sv) changed = true;
		s[idx+x] = sv;
	    }
	}
    }

    check_consistency();
    return changed;
}

bool CCmVariable::setVariableBool(bool value, unsigned idx)
{
    ConsistencyCheck cch(this);
    bool changed = false;
    unsigned pos = idx;
    if (pos < (array_y*array_x))
    {
	if (type == 'b')
	{
	    assert(i[pos] == 0L || i[pos] == 1L);
	    libcm_integer value_i = (value ? 1L : 0L);
	    if (i[pos] != value_i)
	    {
		i[pos] = value_i;
		changed = true;
	    }
	} else {
	    cms_ns_if_print("libcm",1, "CmVariable setVariableBool \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
	}
    } else {
	cms_ns_if_print("libcm",1, "CmVariable setVariableBool \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
    }
    return changed;
}

bool CCmVariable::setVariableBool(bool value, unsigned idx_y, unsigned idx_x)
  { return setVariableBool(value, (idx_y * array_x) + idx_x); }

bool CCmVariable::setVariableInt(libcm_integer value, unsigned idx)
{
    ConsistencyCheck cch(this);
    // libcm_integer my_value = value;
    if (type == 'i')
    {
	if (value < i_min)
	{
	    cms_ns_if_print("libcm",2, "CmVariable %s: setVariableInt %" PRId64" < minimum=%" PRId64", adjusted",
			    name.c_str(),
			    value,
			    i_min);
	    value = i_min;
	}
	if (value > i_max)
	{
	    cms_ns_if_print("libcm",2, "CmVariable %s: setVariableInt %" PRId64" > maximum=%" PRId64", adjusted",
			    name.c_str(),
			    value,
			    i_max);
	    value = i_max;
	}
    }
    unsigned pos = idx;
    if (pos < (array_y*array_x))
    {
	if (type == 'i' || type=='b' || type=='e')
	{
	    if (type == 'b')
	    {
		cms_ns_if_print("libcm",2, "CmVariable %s: setVariableInt %" PRId64" on a Boolean variable, converted",
				name.c_str(),
				value);
		value = (value != 0L);
	    }
	    if (type == 'e')
	    {
		if (!contains(enum_values, value))
		{
		    cms_ns_if_print("libcm", 1, "CmVariable %s: setVariableInt %" PRId64" on an Enum variable, out of range, ignoring",
				    name.c_str(),
				    value);
		    return false;
		}
		else
		{
		    cms_ns_if_print("libcm", 2, "CmVariable %s: setVariableInt %" PRId64" on an Enum variable, converted",
				    name.c_str(),
				    value);
		}
	    }
	    if (i[pos] != value)
	    {
		i[pos] = value;
		return true;
	    }
	}
	else
	{
	    cms_ns_if_print("libcm",1, "CmVariable %s: setVariableInt %" PRId64" on a type '%c', ignoring", name.c_str(), value, type);
	}
    }
    else
    {
	cms_ns_if_print("libcm",1, "CmVariable setVariableInt \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
    }
    return false;
}

bool CCmVariable::setVariableInt(libcm_integer value, unsigned idx_y, unsigned idx_x)
  { return setVariableInt(value, (idx_y * array_x) + idx_x); }




bool CCmVariable::setVariableFloat(double value, unsigned idx)
{
    ConsistencyCheck cch(this);

  // double my_value = value;
  if (value < f_min) value = f_min;
  if (value > f_max) value = f_max;
  bool changed = false;
  unsigned pos = idx;
  if (pos < (array_y*array_x))
  {
    if (type == 'f')
    {
      if (f[pos] != value)
      {
        f[pos] = value;
        changed = true;
      }
    } else {
      cms_ns_if_print("libcm",1, "CmVariable setVariableFloat \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
    }
  } else {
    cms_ns_if_print("libcm",1, "CmVariable setVariableFloat \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
  }
  return changed;
}

bool CCmVariable::setVariableFloat(double value, unsigned idx_y, unsigned idx_x)
  { return setVariableFloat(value, (idx_y * array_x) + idx_x); }


bool CCmVariable::setVariableString(std::string value, unsigned idx)
{
    
    ConsistencyCheck cch(this);
  bool changed = false;
  unsigned pos = idx;
  if (pos < (array_y*array_x))
  {
    if (type == 's')
    {
      if (s[pos] != value)
      {
        s[pos] = value;
        changed = true;
      }
    } else {
      cms_ns_if_print("libcm",1, "CmVariable setVariableString \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
    }
  } else {
    cms_ns_if_print("libcm",1, "CmVariable setVariableString \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
  }
  return changed;
}


bool CCmVariable::setVariableString(std::string value, unsigned idx_y, unsigned idx_x)
  { return setVariableString(value, (idx_y * array_x) + idx_x); }


bool CCmVariable::setVariableEnum(int64_t value, unsigned idx)
{
    ConsistencyCheck cch(this);

  bool changed = false;
  unsigned pos = idx;
  if (pos < (array_y*array_x))
  {
    if ((type == 'e')||(type == 'i')||(type == 'b'))
    {
      bool found = false;
      std::map <int64_t, std::string>::iterator dit = enum_values.find(value);
      if (dit != enum_values.end()) found = true;
      if (found)
      {
        if (i[pos] != value)
        {
          i[pos] = value;
          changed = true;
        }
      } else {
        cms_ns_if_print("libcm",1, "CmVariable setVariableEnum \"%s\"  ... undefined value %" PRId64" ",name.c_str(),value);
      }
    } else {
      cms_ns_if_print("libcm",1, "CmVariable setVariableEnum \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
    }
  } else {
    cms_ns_if_print("libcm",1, "CmVariable setVariableEnum \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
  }
  return changed;
}


bool CCmVariable::setVariableEnum(int64_t value, unsigned idx_y, unsigned idx_x)
  { return setVariableEnum(value, (idx_y * array_x) + idx_x); }


bool CCmVariable::setVariableEnum(std::string value, unsigned idx)
{
    ConsistencyCheck cch(this);
    bool changed = false;
    unsigned pos = idx;
    if (pos < (array_y*array_x))
    {
	if (type == 'e')
	{
	    libcm_integer value_i = 0;
	    bool found = false;
	    std::map <std::string, int64_t>::iterator dit = enum_strings.find(value);
	    if (dit != enum_strings.end())
	    {
		value_i = dit->second;
		found = true;
	    }
	    if (found)
	    {
		if (i[pos] != value_i)
		{
		    i[pos] = value_i;
		    changed = true;
		}
	    } else {
		cms_ns_if_print("libcm",1, "CmVariable setVariableEnum \"%s\"  ... undefined value \"%s\"",name.c_str(),value.c_str());
		    }
	} else {
	    cms_ns_if_print("libcm",1, "CmVariable setVariableEnum \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
		}
    } else {
	cms_ns_if_print("libcm",1, "CmVariable setVariableEnum \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
	    }
    return changed;
}

bool CCmVariable::setVariableEnum(std::string value, unsigned idx_y, unsigned idx_x)
  { return setVariableEnum(value, (idx_y * array_x) + idx_x); }



bool CCmVariable::getVariableBool(bool &value, unsigned idx) const
{
    bool defined = false;
    unsigned pos = idx;
    if (pos < (array_y*array_x))
    {
	if (type == 'b')
	// if ((type == 'b') || (type == 'i') || (type == 'e'))
	{
	    assert(i[pos] == 0L || i[pos] == 1L);
	    value = (i[pos] ? true : false);
	    defined = true;
	}
	else
	{
	    cms_ns_if_print("libcm",1, "CmVariable getVariableBool \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
	}
    }
    else
    {
	cms_ns_if_print("libcm",1, "CmVariable getVariableBool \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
    }
    return defined;
}


bool CCmVariable::getVariableBool(bool &value, unsigned idx_y, unsigned idx_x) const
{ return getVariableBool(value, (idx_y * array_x) + idx_x); }

bool CCmVariable::getVariableInt(libcm_integer &value, unsigned idx) const
{
    bool defined = false;
    unsigned pos = idx;
    if (pos < (array_y*array_x))
    {
	if ((type == 'b') || (type == 'i') || (type == 'e'))
	{
	    if (type == 'b' || type == 'e')
		cms_ns_if_print("libcm",1, "CmVariable getVariableInt \"%s\"  ... incorrect type \"%c\", converting to int",name.c_str(),type);
	    value = i[pos];
	    defined = true;
	}
	else
	{
	    cms_ns_if_print("libcm",1, "CmVariable getVariableInt \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
	}
    }
    else
    {
	cms_ns_if_print("libcm",1, "CmVariable getVariableInt \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
    }
    return defined;
}

uint64_t convert_deprecated_to_uint(int64_t value)
{
    if (value < 0)
    {
        cms_ns_if_print("libcm", 1, "Called deprecated getVariableInt with uint64_t: cannot represent negative value");
        return 0;
    }
    else
    {
        return value;
    }
}

bool CCmVariable::getVariableInt   (uint64_t    &value, unsigned idx) const
{
    int64_t x;
    bool ok = getVariableInt(x, idx);
    if (ok)
        value = convert_deprecated_to_uint(x);
    return ok;
}


bool CCmVariable::getVariableInt(libcm_integer &value, unsigned idx_y, unsigned idx_x) const
  { return getVariableInt(value, (idx_y * array_x) + idx_x); }

bool CCmVariable::getVariableInt(uint64_t    &value, unsigned idx_y, unsigned idx_x) const
{
    int64_t x;
    bool ok = getVariableInt(x, idx_y, idx_x);
    if (ok)
        value = convert_deprecated_to_uint(x);
    return ok;
}

bool CCmVariable::getVariableFloat(double &value, unsigned idx) const
{
    
    bool defined = false;
    unsigned pos = idx;
    if (pos < (array_y*array_x))
    {
	if (type == 'f')
	{
	    value = f[pos];
	    defined = true;
	} else {
	    cms_ns_if_print("libcm",1, "CmVariable getVariableFloat \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
	}
    } else {
	cms_ns_if_print("libcm",1, "CmVariable getVariableFloat \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
    }
    return defined;
}

bool CCmVariable::getVariableFloat(double &value, unsigned idx_y, unsigned idx_x) const
  { return getVariableFloat(value, (idx_y * array_x) + idx_x); }

bool CCmVariable::getVariableString(std::string &value, unsigned idx) const
{
    
  bool defined = false;
  unsigned pos = idx;
  if (pos < (array_y*array_x))
  {
    if (type == 's')
    {
      value = s[pos];
      defined = true;
    } else {
      cms_ns_if_print("libcm",1, "CmVariable getVariableString \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
    }
  } else {
    cms_ns_if_print("libcm",1, "CmVariable getVariableString \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
  }
  return defined;
}

bool CCmVariable::getVariableString(std::string &value, unsigned idx_y, unsigned idx_x) const
  { return getVariableString(value, (idx_y * array_x) + idx_x); }

bool CCmVariable::getVariableEnum(int64_t &value, unsigned idx) const
{
  bool defined = false;
  unsigned pos = idx;
  if (pos < (array_y*array_x))
  {
    if (type == 'e')
    {
      value = i[pos];
      defined = true;
    } else {
      cms_ns_if_print("libcm",1, "CmVariable getVariableEnum \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
    }
  } else {
    cms_ns_if_print("libcm",1, "CmVariable getVariableEnum \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
  }
  return defined;
}

bool CCmVariable::getVariableEnum(int64_t &value, unsigned idx_y, unsigned idx_x) const
  { return getVariableEnum(value, (idx_y * array_x) + idx_x); }

bool CCmVariable::getVariableEnum(std::string &value, unsigned idx) const
{
  bool defined = false;
  unsigned pos = idx;
  if (pos < (array_y*array_x))
  {
    if (type == 'e')
    {
      std::map <int64_t, std::string>::const_iterator dit = enum_values.find(i[pos]);
      if (dit != enum_values.end())
      {
        value = dit->second;
        defined = true;
      } else {
	  cms_ns_if_print("libcm",1, "CmVariable getVariableEnum \"%s\"  ... undefined value \"%" PRId64"\"",name.c_str(),i[pos]);
      }
    } else {
      cms_ns_if_print("libcm",1, "CmVariable getVariableEnum \"%s\"  ... incorrect type \"%c\"",name.c_str(),type);
    }
  } else {
    cms_ns_if_print("libcm",1, "CmVariable getVariableEnum \"%s\" ... array out of boundary ... RQ pos is pos=%d  ... ARRAY SIZE %d(y=%d,x=%d)",name.c_str(), idx, (array_y * array_x),array_y, array_x);
  }
  return defined;
}

bool CCmVariable::getVariableEnum(std::string &value, unsigned idx_y, unsigned idx_x) const
  { return getVariableEnum(value, (idx_y * array_x) + idx_x); }


bool CCmVariable::setVALUES(char **item, std::list <unsigned> *ProtectedIndexes)
{
    ConsistencyCheck cch(this);
  if (item == NULL) return false;
  bool changed=false;
  if (isArray())
  {
    unsigned i = 0;
    unsigned ptr_array_x = 0, ptr_array_y = 0;
    unsigned arr_size = array_x * array_y;
//  0_5:0=0, 0_5:1=1   ;;; 0,0,0,0,0,0,0,0,0,0,0,0
    while (item[i])
    {
      char **item_1 = cm_string_strsplit_outside_string_and_brackets(item[i], "=", "[](){}", 0);
      if ((item_1)&&(item_1[0])&&(item_1[1]))
      {
        //  1:1=3,   0_5:0=0, 0_5:1=1   ;;; 0,0,0,0,0,0,0,0,0,0,0,0
        char **item_2 = cm_string_strsplit_outside_string(item_1[0], ":", 0);
        if (item_2)
        {
          int y_pos = -1, y_count = 1;
          int x_pos = -1, x_count = 1;
          if (item_2[0]) // y
          {
            char **item_3 = cm_string_strsplit_outside_string(item_2[0], "_", 0);
            if ((item_3) && (item_3[0]))
            {
              y_pos = cm_string_get_value_int(item_3[0]);
              if (item_3[1]) y_count = (cm_string_get_value_int(item_3[1]) ) ;
//              if (item_3[1]) y_count = (cm_string_get_value_int(item_3[1]) - y_pos) + 1;
            }
            if (item_3) g_strfreev(item_3);
          }
          if (item_2[1]) // x
          {
            char **item_3 = cm_string_strsplit_outside_string(item_2[1], "_", 0);
            if ((item_3) && (item_3[0]))
            {
              x_pos = cm_string_get_value_int(item_3[0]);
              if (item_3[1]) x_count = (cm_string_get_value_int(item_3[1]) );
//              if (item_3[1]) x_count = (cm_string_get_value_int(item_3[1]) - x_pos) + 1;
            }
            if (item_3) g_strfreev(item_3);
          }
          g_strfreev(item_2);

//          if (x_pos == -1) x_pos = ptr_array_x;
          if (x_pos == -1)
          {
            if (array_x > 1)
            {
              x_pos = y_pos;
              y_pos = 0;
              x_count = y_count;
              y_count = 1;
            } else {
              x_pos = y_pos;
              y_pos = 0;
            }
          }
          for (int y = 0; y < y_count; y++)
          {
            ptr_array_y = y + y_pos;
            ptr_array_x = x_pos;
            if (ptr_array_x >= array_x) { ptr_array_y += ptr_array_x / array_x; ptr_array_x %= array_x; }

            unsigned pos = (ptr_array_y * array_x) + ptr_array_x;
            if (pos < arr_size)
            {
              unsigned x_cnt = x_count;
//              if ((ptr_array_x + (x_cnt - 1)) >= array_x) x_cnt = (array_x - ptr_array_x) + 1;
              if ((pos + (x_cnt - 1)) >= arr_size) x_cnt = (arr_size - pos) + 1;
              if (setValues(std::string(item_1[1]), pos, x_cnt, ProtectedIndexes)) changed = true;
              ptr_array_x += (x_cnt-1);
            } else {
              cms_ns_if_print("libcm",1, "CmVariable set_value \"%s\" ... array out of boundary ... RQ pos is y=%d,x=%d  ... ARRAY SIZE y=%d,x=%d  ... (%d>=%d)",name.c_str(), ptr_array_y, ptr_array_x, array_y, array_x, pos , arr_size);
            }
          }
          ptr_array_x++; if (ptr_array_x >= array_x) { ptr_array_y += ptr_array_x / array_x; ptr_array_x %= array_x; }
        }
      } else {
        // 0,0,0,0,0,0,0,0,0,0,0,0
        unsigned pos = (ptr_array_y * array_x) + ptr_array_x;
        if (pos < arr_size)
        {
          if (setValues(std::string(item[i]), pos, 1, ProtectedIndexes)) changed = true;
        } else {
          cms_ns_if_print("libcm",1, "CmVariable set_value \"%s\" ... array out of boundary ... RQ pos is y=%d,x=%d  ... ARRAY SIZE y=%d,x=%d",name.c_str(), ptr_array_y, ptr_array_x, array_y, array_x);
        }
        ptr_array_x++; if (ptr_array_x >= array_x) { ptr_array_y += ptr_array_x / array_x; ptr_array_x %= array_x; }
      }
      if (item_1)
      {
        g_strfreev(item_1);
      }
      i++;
    }
  } else {
    if (item[0])
    {
      if (setValues(std::string(item[0]),0)) changed = true; // default value
    }
  }
  check_consistency();
  return changed;
}

bool CCmVariable::setVALUES(std::string inp, std::list <unsigned> *ProtectedIndexes)
{
    ConsistencyCheck cch(this);    
   debug_printf("setVALUES: %s", inp.c_str());
    bool retval = false;
    char *command = cm_string_strdup_nospace_outside_string(inp.c_str());
    if (command)
    {
        char **item = cm_string_strsplit_outside_string_and_brackets(command, ",", "[](){}", 0);
        if (item)
        {
            retval = setVALUES(item, ProtectedIndexes);
            g_strfreev(item);
        }
        g_free(command);
    }
    check_consistency();
    return retval;
}



std::string CCmVariable::getVALUES(bool OnlyChanged) const
{
  bool enable_group = true;
  if (isArray())
  {  // first version : simply all  
    std::string retstr = std::string("");
    if (OnlyChanged)
    {
      unsigned arr_size = array_x * array_y;
      unsigned last_used_index = 0xffff;
      bool first_item = true;
      for (unsigned i=0; i<arr_size; i++)
      {
        bool use = false;
        if (EventFields.size()>0)
        {
          for (std::list <unsigned>::const_iterator it = EventFields.begin(); ((it != EventFields.end()) && (!use)); it++)
          {
            if ((*it)==i) use = true;
          }
        } else use = true;
        if (use)
        {
          char WStr1[50];
          if (!first_item) retstr += std::string(",");
          if ((last_used_index+1)!=i)
          {
            sprintf(WStr1,"%d=",i);
            retstr += std::string(WStr1);
          }
          retstr += getValue(i);
          last_used_index = i;
          first_item = false;
        }
      }
    } else {
      unsigned arr_size = array_x * array_y;
      bool first_item = true;
      for (unsigned i=0; i<arr_size; i++)
      {
        if (!first_item) retstr += std::string(",");
        std::string myValue = getValue(i);
        char multi_str[50]; multi_str[0] = 0;
        if (enable_group)
        {
          unsigned same_values = 0;
          bool different_value = false;
          for (unsigned j = i+1; (j < arr_size) && (!different_value); j++)
          {
            if (myValue == getValue(j))
            {
              same_values ++;
            } else {
              different_value = true;
            }
          }
          if (same_values > 1)
          {
            sprintf(multi_str,"%d_%d=",i,same_values+1); // val=(0,5)=0,8369,(7,23)=0,(31,3)=99  ....  0,0,0,0,0,0,8369,...
            i += same_values;
          }
        }
        retstr += std::string(multi_str) + myValue;
        first_item = false;
      }
    }


    // !!!!!!!!!! POZDEJI
    // bylo by mozne zkouset ruzne metody a porovnavat vyslednou velikost
    // take by bylo potreba do budoucna tam dat moznost porovnani predchozi hodnoty a masku --> vysledna maska .... encode
    
    return retstr;
  } else {
    return getValue(0);
  }
}

// uint8_t     flags; // b0=Signed, b1=External, b2=Writable, b3=SaveFile, b4=?, b5=?, b6=?, b7=PriorityChanged
uint8_t read_flags(char const *str)
{
    uint8_t flags = 0; 
    if ((strlen(str)>0) && ((str[0]=='Y') || (str[0]=='y'))) flags |= (1 << CM_VARIABLE_FLAG_EXTERNAL);
    if ((strlen(str)>1) && ((str[1]=='Y') || (str[1]=='y'))) flags |= (1 << CM_VARIABLE_FLAG_WRITABLE);
    if ((strlen(str)>2) && ((str[2]=='Y') || (str[2]=='y'))) flags |= (1 << CM_VARIABLE_FLAG_SAVEFILE);
    if ((strlen(str)>3) && ((str[3]=='Y') || (str[3]=='y'))) flags |= (1 << CM_VARIABLE_FLAG_VIRTUAL);
    return flags;
}

void read_array_and_default_value(char const *str, unsigned *array_y, unsigned *array_x, std::string *def_value)
{
    *array_x = 1; // default
    *array_y = 1; // default
    *def_value = ""; // default
    char **it_arr1 = cm_string_strsplit_outside_string_and_brackets(str, "=", "[](){}", 0);
    if (it_arr1)
    {
	if ((it_arr1[0]) && (strlen(it_arr1[0])>0))
	{
	    char **it_arr2 = cm_string_strsplit_outside_string(it_arr1[0], ":", 0);
	    if (it_arr2)
	    {
		if ((it_arr2[0]) && (strlen(it_arr2[0])>0))
		{
		    *array_y = cm_string_get_value_int(it_arr2[0]);
		    if ((it_arr2[1]) && (strlen(it_arr2[1])>0))
		    {
			*array_x = cm_string_get_value_int(it_arr2[1]);
		    }
		}
		g_strfreev(it_arr2);
	    }
	    if ((it_arr1[1]) && (strlen(it_arr1[1])>0))
	    {
		*def_value = std::string(it_arr1[1]);
	    }
	}
	g_strfreev(it_arr1);
    }
}

int64_t get_int64_or_default(std::string input, int64_t default_val)
{
    if (input == "")
	return default_val;
    return cm_string_get_value_int64(input.c_str());
}

// item[0] = name
// item[1] = priority
// item[2] = MaxUpdateFreq
// item[3] = flags
// item[4] = array
// item[5]... Limits + Values
std::string CCmVariable::defineFromArray(char **item, std::string inp)
{
    ConsistencyCheck cch(this);    
    if ((item[0]) && (item[1]) && (item[2]) && (item[3]) && (item[4]))
    {
	enum_values.clear();
	enum_strings.clear();
	enum_sorting.clear();

	// item[0]: Name
	name = std::string(item[0]);
	if (!libCM_is_valid_variable_name(item[0]))
	    return std::string("Invalid variable name: ") + item[0];

	// item[1]: Priority
	priority = cm_string_get_value_int(item[1]);

	// item[2]: MaxUpdateFreq
	MaxUpdateFreq = cm_string_get_value_double(item[2]);

	// item[3] : Flags
	flags = read_flags(item[3]);

	// item[5]: Read array + default value as string
	std::string def_value;
	read_array_and_default_value(item[5], &array_y, &array_x, &def_value);

	unsigned new_array_size = 1;
	if (isArray()) new_array_size = (array_x * array_y) + 1;
	unsigned next_item_idx = 6;

	if ((item[4][0] == 'i') || (item[4][0] == 'I'))
	{
	    type = 'i'; // int
	    i.resize(new_array_size);
	    if ((item[next_item_idx+0]) && (item[next_item_idx+1])&&(item[next_item_idx+2]))
	    {
		i_min  = get_int64_or_default(item[next_item_idx+0], INT64_MIN);
		i_max  = get_int64_or_default(item[next_item_idx+1], INT64_MAX);
		i_step = cm_string_get_value_int64(item[next_item_idx+2]);
		next_item_idx += 3;
	    } else {
		return std::string("VARIABLE: NO ENOUGH PARAMS FOR INT ...")+inp;
	    }
	} else if ((item[4][0] == 'e') || (item[4][0] == 'E'))
	{
	    type = 'e'; // enum
	    i.resize(new_array_size);
	    if ((item[next_item_idx+0]))
	    {
		// 0="M1":1="M2":3="M3/A":6="MC":23="empty"
		enum_values.clear();
		enum_strings.clear();
		enum_sorting.clear();
		std::string def_value = std::string("");
		char **it_en1 = cm_string_strsplit_outside_string_and_brackets(item[next_item_idx+0], ":", "[](){}", 0);
		if (it_en1)
		{
		    bool first = true;
		    bool sorting_need = false;
		    int64_t LastValue = 0;
		    unsigned en_i = 0;
		    while (it_en1[en_i])
		    {
			char **it_en2 = cm_string_strsplit_outside_string_and_brackets(it_en1[en_i], "=", "[](){}", 0);
			if (it_en2)
			{
			    if ((it_en2[0]) && (it_en2[1]))
			    {
				gchar * it_en2_1 = cm_string_interpret_first_string(it_en2[1]);
				if (it_en2_1)
				{
				    int idx  = cm_string_get_value_int(it_en2[0]);
				    enum_values[idx] = std::string(it_en2_1);
				    enum_strings[std::string(it_en2_1)] = idx;
				    g_free(it_en2_1);
				    if ((!first) && ((int64_t)idx <= LastValue)) sorting_need = true;
				    enum_sorting.push_back(idx);
				    LastValue = idx;
//                 debug_printf("  def : %d, %d sorting=%s",(int)idx,(int)LastValue,(sorting_need?"yes":"no"));
				}
			    }
			    g_strfreev(it_en2);
			}
			en_i++;
			first = false;
		    }
		    if (!sorting_need) enum_sorting.clear();
//         debug_printf("**DEF %s sorting=%s",name.c_str(),(sorting_need?"YES":"n"));
		    g_strfreev(it_en1);
		}
		next_item_idx += 1;
	    } else {
		return std::string("VARIABLE: NO ENOUGH PARAMS FOR ENUM ...")+inp;
	    }
	} else if ((item[4][0] == 'f') || (item[4][0] == 'F'))
	{
	    type = 'f'; // float
	    f.resize(new_array_size);
	    if ((item[next_item_idx+0]) && (item[next_item_idx+1])&&(item[next_item_idx+2]))
	    {
		f_min  = cm_string_get_value_double(item[next_item_idx+0]);
		f_max  = cm_string_get_value_double(item[next_item_idx+1]);
		f_step = cm_string_get_value_double(item[next_item_idx+2]);
		recalculate_max_decimals();
		next_item_idx += 3;
	    } else {
		return std::string("VARIABLE: NO ENOUGH PARAMS FOR FLOAT ...")+inp;
	    }
	} else if ((item[4][0] == 's') || (item[3][0] == 'S'))
	{
	    type = 's'; // string
	    s.resize(new_array_size);
	    if ((item[next_item_idx+0]))
	    {
		s_maxsize = cm_string_get_value_int(item[next_item_idx+0]);
		next_item_idx += 1;
	    } else {
		return std::string("VARIABLE: NO ENOUGH PARAMS FOR STRING ...")+inp;
	    }
	} else if ((item[4][0] == 'b') || (item[4][0] == 'B'))
	{
	    type = 'b'; // bool
	    i.resize(new_array_size);
	} else {
	    return std::string("VARIABLE: UNKNOWN TYPE (") + std::string(item[4]) + std::string(") ...")+inp;
	}

	if (new_array_size > 1) // array
	{
	    setValues(def_value,0,new_array_size); // default value
	    if (item[next_item_idx+0]) setVALUES(item+next_item_idx+0);
	} else {
	    if (item[next_item_idx+0]) // value(s)
	    {
		setVALUES(item+next_item_idx+0);
	    } else {
		return std::string("VARIABLE: NO ENOUGH PARAMS : VALUE EMPTY ...")+inp;
	    }
	}
    } else return std::string("VARIABLE: NO ENOUGH PARAMS ")+inp;
    if (name.size()==0)
    {
	return std::string("VARIABLE: EMPTY NAME ...")+inp;
    }

    return std::string(); // empty
}


std::string CCmVariable::defineFromString(std::string inp)
{
    ConsistencyCheck cch(this);    
  std::string retval = std::string();
  char *command = cm_string_strdup_nospace_outside_string(inp.c_str());
  if (command)
  {
    char **item = cm_string_strsplit_outside_string_and_brackets(command, ",", "[](){}", 0);
    if (item)
    {
      retval = defineFromArray(item, inp);
      g_strfreev(item);
    }
    g_free(command);
  }
  check_consistency();
  return retval;
}

std::string CCmVariable::toStringDefine() const
{
    std::string retstr;
    retstr += name;
    retstr += ",";
    retstr += toStringDefineTypeValue();

    return retstr;
}


std::string CCmVariable::toStringDefineTypeValue() const
{
  std::string retstr = std::string("");

  char s_priority[20]; sprintf(s_priority,"%d",priority);
  retstr += std::string(s_priority);

  char s_updfreq[20]; sprintf(s_updfreq,"%s",cm_std_string_print_short_float(MaxUpdateFreq).c_str());
  retstr += std::string(",") + std::string(s_updfreq);


  // External , Writable, SaveFile
  char s_flags[20]; sprintf(s_flags,"%c%c%c", 
       (((flags >> CM_VARIABLE_FLAG_EXTERNAL) & 1) ? 'Y' : 'N'),
       (((flags >> CM_VARIABLE_FLAG_WRITABLE) & 1) ? 'Y' : 'N'),
       (((flags >> CM_VARIABLE_FLAG_SAVEFILE) & 1) ? 'Y' : 'N') );
  if (haveFlagVirtual()) sprintf(s_flags+strlen(s_flags),"Y");
  retstr += std::string(",") + std::string(s_flags);

  char s_type[20]; sprintf(s_type,"%c", type);
  retstr += std::string(",") + std::string(s_type);

  std::string arr_def = std::string("");
  if (isArray())
  {
    char s_arr_def[40];
    if (array_x<2)
    {
      sprintf(s_arr_def,"%d",array_y);
    } else {
      sprintf(s_arr_def,"%d:%d",array_y,array_x);
    }
    arr_def = std::string(s_arr_def) + std::string("=") + getValue(array_y*array_x); // default value
  }
  retstr += std::string(",") + arr_def;

  if (type=='b')
  {
  } else if ((type=='i'))
  {
    char s_minmaxstep[60];
    sprintf(s_minmaxstep,"%" PRId64 ",%" PRId64 ",%" PRId64 "",i_min,i_max,i_step);
    retstr += std::string(",") + std::string(s_minmaxstep);
  } else if (type=='f')
  {
    char s_minmaxstep[100];
    sprintf(s_minmaxstep,"%s,%s,%s",
	    cm_std_string_print_short_float(f_min).c_str(),
	    cm_std_string_print_short_float(f_max).c_str(),
	    cm_std_string_print_short_float(f_step).c_str());
    retstr += std::string(",") + std::string(s_minmaxstep);
  } else if (type=='s')
  {
    char s_s_maxsize[40]; sprintf(s_s_maxsize,"%d",s_maxsize);
    retstr += std::string(",") + std::string(s_s_maxsize);
  } else if (type=='e')
  {
            // 0="M1":1="M2":3="M3/A":6="MC":23="empty"
    retstr += std::string(",")+generateEnumDefinition();
  }

  retstr += std::string(",") + getVALUES();

  return retstr;
}

std::string CCmVariable::generateEnumDefinition() const
{
     // 0="M1":1="M2":3="M3/A":6="MC":23="empty"
  std::string retstr;
  bool first = true;
  if (enum_sorting.empty())
  {
    for (std::map<int64_t, std::string>::const_iterator it=enum_values.begin(); it!=enum_values.end(); it++)
    {
	std::string s_val = escape_special_chars(it->second);
      // gchar * s_val = cm_string_escape_string(it->second.c_str(), "#\n\"\r\t\\", true);
      // if (s_val)
      // {
        if (!first) retstr += std::string(":");
        char s_idx[40]; sprintf(s_idx,"%" PRId64 "",it->first);
        retstr += std::string(s_idx) + std::string("=") + "\"" + std::string(s_val) + "\"";
        // g_free(s_val);
        first = false;
      // }
    }
//   debug_printf("GEN:empty %s %s",name.c_str(),retstr.c_str());
  } else {
    for (std::list<int64_t>::const_iterator its=enum_sorting.begin(); its!=enum_sorting.end(); its++)
    {
      std::map<int64_t, std::string>::const_iterator it=enum_values.find(*its);
      std::string s_val = escape_special_chars(it->second);
      // gchar * s_val = cm_string_escape_string(it->second.c_str(), "#\n\"\r\t\\", true);
      // if (s_val)
      // {
        if (!first) retstr += std::string(":");
        char s_idx[40]; sprintf(s_idx,"%" PRId64 "",it->first);
        retstr += std::string(s_idx) + std::string("=") + "\"" + std::string(s_val) + "\"";
        // g_free(s_val);
        first = false;
      // }
    }
//   debug_printf("GEN:NONEMPTY %s %s",name.c_str(),retstr.c_str());
  }
  return retstr;
}

std::string CCmVariable::fullPrint(double ActTime) const
{
  bool enable_group = true;

  std::string retstr = std::string("");

  retstr += name;

  if (isArray())
  {
    char s_arr_def[40];
    if (array_x>1)
    {
      sprintf(s_arr_def,"%d,%d",array_y,array_x);
    } else {
      sprintf(s_arr_def,"%d",array_y);
    }
    retstr += std::string("[") + s_arr_def + std::string("]");
  }


//  char s_priority[20]; sprintf(s_priority,"%d",priority);
//  retstr += std::string(",") + std::string(s_priority);
//  char s_updfreq[20]; sprintf(s_updfreq,"%g",MaxUpdateFreq);
//  retstr += std::string(",") + std::string(s_updfreq);

  // External , Writable, SaveFile

  char s_flags[20]; sprintf(s_flags,"flags=%c%c%c", 
       (((flags >> CM_VARIABLE_FLAG_EXTERNAL) & 1) ? 'Y' : 'N'),
       (((flags >> CM_VARIABLE_FLAG_WRITABLE) & 1) ? 'Y' : 'N'),
       (((flags >> CM_VARIABLE_FLAG_SAVEFILE) & 1) ? 'Y' : 'N') );
  if (haveFlagVirtual()) sprintf(s_flags+strlen(s_flags),"Y");
  retstr += std::string(",") + std::string(s_flags);

  char s_type[20]; sprintf(s_type,"(%c)", type);


         if (type=='b')
  {
    retstr += std::string(",") + std::string("(bool)");
  } else if ((type=='i'))
  {
    char s_minmaxstep[200];
    sprintf(s_minmaxstep,"%" PRId64 ",%" PRId64 ",%" PRId64 "",i_min,i_max,i_step);
    retstr += std::string(",") + std::string("(int)") + std::string(",") + std::string(s_minmaxstep);
  } else if (type=='f')
  {
    retstr += std::string(",") + std::string("(float)") + std::string(",") + cm_std_string_print_short_float(f_min) + std::string(",") + cm_std_string_print_short_float(f_max) + std::string(",") + cm_std_string_print_short_float(f_step);
  } else if (type=='s')
  {
    char s_s_maxsize[40]; sprintf(s_s_maxsize,"%d",s_maxsize);
    retstr += std::string(",") + std::string("(string)") + std::string(",") + std::string(s_s_maxsize);
  } else if (type=='e')
  {
            // 0="M1":1="M2":3="M3/A":6="MC":23="empty"
    retstr += std::string(",") + std::string("(enum)") + std::string(",");
    bool first = true;
    for (std::map<int64_t, std::string>::const_iterator it=enum_values.begin(); it!=enum_values.end(); it++)
    {
	std::string s_val = escape_special_chars(it->second);
      // gchar * s_val = cm_string_escape_string(it->second.c_str(), "#\n\"\r\t\\", true);
      // if (s_val)
      // {
        if (!first) retstr += std::string(":");
        char s_idx[40]; sprintf(s_idx,"%" PRId64 "",it->first);
        retstr += std::string(s_idx) + std::string("=") + std::string(s_val);
        // g_free(s_val);
        first = false;
      // }
    }
  } else {
    retstr += std::string(",") + std::string(s_type);
  }

  if ((ActTime != 0.0) &&  (modif_time != 0.0))
  {
    char ss_age[150]; sprintf(ss_age,"age=%s",cm_std_string_print_short_float((double)(ActTime - modif_time)).c_str());
    retstr += std::string(",") + std::string(ss_age);
  }

  if (isArray())
  {  // first version : simply all  
      retstr += std::string(",val=") ;
      unsigned arr_size = array_x * array_y;
      bool first_item = true;
      for (unsigned i=0; i<arr_size; i++)
      {
        if (!first_item) retstr += std::string(",");
        std::string myValue = getValue(i);
        char multi_str[50]; multi_str[0] = 0;
        if (enable_group)
        {
          unsigned same_values = 0;
          bool different_value = false;
          for (unsigned j = i+1; (j < arr_size) && (!different_value); j++)
          {
            if (myValue == getValue(j))
            {
              same_values ++;
            } else {
              different_value = true;
            }
          }
          if (same_values > 1)
          {
            sprintf(multi_str,"{%dx}",same_values+1);
//            sprintf(multi_str,"(%d,%d)=",i,same_values);
            i += same_values;
          }
        }
        retstr +=  myValue + std::string(multi_str);
        first_item = false;
      }
  } else {
    retstr += std::string(",val=") + getValue(0);
  }
  return retstr;
}

void CCmVariable::addEventField(unsigned F)
{
    ConsistencyCheck cch(this);
    bool found = false;
    for (std::list<unsigned>::iterator it = EventFields.begin(); it != EventFields.end(); it++)
    {
	if ((*it) == (F)) found = true;
    }
    if (!found) EventFields.push_back(F);

}

void CCmVariable::addEventFields(std::list<unsigned> *_EventFields)
{
    ConsistencyCheck cch(this);
    for (std::list<unsigned>::iterator it = _EventFields->begin(); it != _EventFields->end(); it++)
    {
	addEventField((*it));
    }
}

void libCM_LoadConfigVariables(CMSConfig *config, VarMap *LoadedVariables, std::string *cm_name)
{
    if (LoadedVariables == NULL) return;
    std::string const &MyLibCM_name = cm_name ? *cm_name : "";
    std::string name;
    std::string line;
    while (CMSConfigFindNextItem2(*config, name))
    {
	line = CMSConfigGetLine2(*config);
	CCmVariable var;
	std::string error_str = var.defineFromString(name + std::string(",") + line);
	if (error_str.empty())
	{
	    if (contains(*LoadedVariables, var.getName()))
		WORKER_DEBUG("libcm", 1, "Load config variable: Multiple definitions of variable %s. Fix the config file!", var.getName().c_str());
	    else
		(*LoadedVariables)[var.getName()] = var;
	}
	else
	{
	    WORKER_DEBUG("libcm", 1, "Load config variable: %s", error_str.c_str());
	}
    }
}

std::string libCM_ListAllKnownVariablesCM(VarMap &_VariablesCM ,std::string pattern, bool printHeadFoot)
{
  std::string RetVal;
  double ActTime = libCM_GetActualTimeDouble();
  if (printHeadFoot) RetVal += std::string("================================ CM begin ===========================\n");
  for (VarMap::iterator it = _VariablesCM.begin(); it != _VariablesCM.end(); it++)
  {
    bool printIt = true;
    if (pattern.size()>0)
    {
//bool        libCM_isNameMatching(const char *pattern, const char *name);
      printIt = libCM_isNameMatching(pattern, it->second.getName());
    }
    if (printIt) RetVal += std::string("  ") + it->second.fullPrint(ActTime) + std::string("\n");
  }
  if (printHeadFoot) RetVal += std::string("================================ CM end ===========================\n");
  return RetVal;
}

void fill_full_event_fields(CCmVariable &v)
{
    unsigned x_size = v.getArrayX();
    unsigned y_size = v.getArrayY();
    v.EventFields.clear();
    for (unsigned i = 0; i < x_size * y_size; ++i)
        v.EventFields.push_back(i);
}


bool libCM_is_valid_variable_name(std::string const &str)
{
    if (str.empty())
        return false;
    if (!isalpha(str[0]) && str[0] != '_')
        return false;
    for (size_t i = 1; i < str.size(); ++i)
        if (!isalnum(str[0]) && str[0] != '_')
            return false;
    return true;
}


