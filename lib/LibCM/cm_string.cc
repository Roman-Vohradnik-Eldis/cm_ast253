#include "cm_header_internal.hh"
#include "cm_string.hh"
#include "cm_debug.hh"
#include <sstream>

std::string cm_string_format(char const *fmt, ...)
{
    char buffer[1000];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof buffer, fmt, args);
    va_end(args);
    return buffer;
}

gchar** cm_string_strsplit_outside_string_and_brackets(const gchar *inp, const gchar *delimiter, const gchar *brackets,  gint  max_tokens, bool debug_me)
{
  g_return_val_if_fail (inp != NULL, NULL);
  g_return_val_if_fail (delimiter != NULL, NULL);
  g_return_val_if_fail (delimiter[0] != '\0', NULL);

  if (debug_me)debug_printf("INP:%s DELIM:%s BRACKETS=%s",inp,delimiter,brackets);

  struct t_cm_string_split_pair
  {
    guint start;
    guint len;
  };
  guint num_brackets = strlen(brackets) / 2;
  guint *bracket_counter = NULL;
  GList *pair_list = NULL;
  guint pair_list_size = 0;
  gboolean in_string = false;
  gboolean in_bracket = false;
  gint act_size = 0;
  gint LPtr = 0;
  gchar prev_char = ' ';
  gint strlen_inp = strlen(inp);

  if (num_brackets)
  {
    bracket_counter = g_new (guint, num_brackets);
    for (guint k=0; k<num_brackets; k++) bracket_counter[k] = 0;
  }


  for (gint APtr=0;APtr<strlen_inp; APtr++)
  {
    gchar ActChar = inp[APtr];
    if (in_string)
    {
      if ((ActChar=='\"') && (prev_char != '\\')) in_string = false;
      act_size ++;
    } else {
      if (ActChar=='\"')
      {
        in_string = true;
        act_size ++;
      } else {
        gboolean matching = false;
        gboolean brackets_changed = false;
        for (guint k=0; k<(num_brackets*2); k++) if (ActChar == brackets[k])
        {
          if ((k%2)==0)
          {
            bracket_counter[k/2] ++;
          } else {
            if (bracket_counter[k/2] > 0)  bracket_counter[k/2] --;
          }
          brackets_changed = true;
        }
        if (brackets_changed)
        {
          in_bracket = false;
          for (guint k=0; k<(num_brackets); k++) if (bracket_counter[k]) in_bracket=true;
        }
        if (!in_bracket)
        {
          for (guint k=0; k<strlen(delimiter); k++) if (ActChar == delimiter[k]) matching = true;
        }
        if (!matching)
        {
          act_size ++;
        } else {
//          if (act_size)
          {
            t_cm_string_split_pair * split_pair = g_new(t_cm_string_split_pair, 1);
            split_pair->start = LPtr;
            split_pair->len = act_size;
            pair_list = g_list_append(pair_list, split_pair);
            pair_list_size ++;
          }
          act_size = 0;
          LPtr = APtr+1;
        }
      }
    }
    prev_char = ActChar;
  }
//  if (act_size)
  {
    t_cm_string_split_pair * split_pair = g_new(t_cm_string_split_pair, 1);
    split_pair->start = LPtr;
    split_pair->len = act_size;
    pair_list = g_list_append(pair_list, split_pair);
    pair_list_size ++;
  }
  if (bracket_counter)
  {
    g_free(bracket_counter);
  }
  if (pair_list_size)
  {
    gchar ** str_array = g_new (gchar*, pair_list_size + 1);
    guint counter = 0;
    for (GList *link = pair_list; link ; link = g_list_next(link))
    {
      t_cm_string_split_pair * split_pair = (t_cm_string_split_pair *)link->data;
      str_array[counter] = g_strndup(inp + split_pair->start, split_pair->len);
      g_free(split_pair);
      counter++;
    }
    str_array[counter] = NULL;
    g_list_free(pair_list);
    return str_array;
  }
  return NULL;
}

gchar** cm_string_strsplit_outside_string(const gchar *inp, const gchar *delimiter, gint  max_tokens)
{
  g_return_val_if_fail (inp != NULL, NULL);
  g_return_val_if_fail (delimiter != NULL, NULL);
  g_return_val_if_fail (delimiter[0] != '\0', NULL);

  struct t_cm_string_split_pair
  {
    guint start;
    guint len;
  };
  GList *pair_list = NULL;
  guint pair_list_size = 0;
  gboolean in_string = false;
  gint act_size = 0;
  gint LPtr = 0;
  gchar prev_char = ' ';
  gint strlen_inp = strlen(inp);
  for (gint APtr=0;APtr<strlen_inp; APtr++)
  {
    gchar ActChar = inp[APtr];
    if (in_string)
    {
      if ((ActChar=='\"') && (prev_char != '\\')) in_string = false;
      act_size ++;
    } else {
      if (ActChar=='\"')
      {
        in_string = true;
        act_size ++;
      } else {
        gboolean matching = false;
        for (guint k=0; k<strlen(delimiter); k++) if (ActChar == delimiter[k]) matching = true;
        if (!matching)
        {
          act_size ++;
        } else {
//          if (act_size)
          {
            t_cm_string_split_pair * split_pair = g_new(t_cm_string_split_pair, 1);
            split_pair->start = LPtr;
            split_pair->len = act_size;
            pair_list = g_list_append(pair_list, split_pair);
            pair_list_size ++;
          }
          act_size = 0;
          LPtr = APtr+1;
        }
      }
    }
    prev_char = ActChar;
  }
//  if (act_size)
  {
    t_cm_string_split_pair * split_pair = g_new(t_cm_string_split_pair, 1);
    split_pair->start = LPtr;
    split_pair->len = act_size;
    pair_list = g_list_append(pair_list, split_pair);
    pair_list_size ++;
  }
  if (pair_list_size)
  {
    gchar ** str_array = g_new (gchar*, pair_list_size + 1);
    guint counter = 0;
    for (GList *link = pair_list; link ; link = g_list_next(link))
    {
      t_cm_string_split_pair * split_pair = (t_cm_string_split_pair *)link->data;
      str_array[counter] = g_strndup(inp + split_pair->start, split_pair->len);
      g_free(split_pair);
      counter++;
    }
    str_array[counter] = NULL;
    g_list_free(pair_list);
    return str_array;
  }
  return NULL;
}

gchar * cm_string_strdup_nospace_outside_string(const gchar *inp, const gchar *delchars)
{
  gboolean in_string = false;
  gint OPtr=0;
  gchar prev_char = ' ';
  gint strlen_inp = strlen(inp);
  gchar *retstr = (gchar *) g_new (gchar, strlen_inp+1);
  for (gint APtr=0;APtr<strlen_inp; APtr++)
  {
    gchar ActChar = inp[APtr];
    if (in_string)
    {
      if ((ActChar=='\"') && (prev_char != '\\')) in_string = false;
      retstr[OPtr] = ActChar; OPtr++;
    } else {
      if (ActChar=='\"')
      {
        in_string = true;
        retstr[OPtr] = ActChar; OPtr++;
      } else {
        gboolean matching = false;
        for (guint k=0; k<strlen(delchars); k++) if (ActChar == delchars[k]) matching = true;
        if (!matching)
        {
          retstr[OPtr] = ActChar; OPtr++;
        }
      }
    }
    prev_char = ActChar;
  }
  retstr[OPtr] = 0; OPtr++;
  return retstr;
}

gchar * cm_string_strdup_replace_outside_string(const gchar *inp, const gchar *delchars, gchar replace)
{
  gboolean in_string = false;
  gint OPtr=0;
  gchar prev_char = ' ';
  gint strlen_inp = strlen(inp);
  gchar *retstr = (gchar *) g_new (gchar, strlen_inp+1);
  for (gint APtr=0;APtr<strlen_inp; APtr++)
  {
    gchar ActChar = inp[APtr];
    if (in_string)
    {
      if ((ActChar=='\"') && (prev_char != '\\')) in_string = false;
      retstr[OPtr] = ActChar; OPtr++;
    } else {
      if (ActChar=='\"')
      {
        in_string = true;
        retstr[OPtr] = ActChar; OPtr++;
      } else {
        gboolean matching = false;
        for (guint k=0; k<strlen(delchars); k++) if (ActChar == delchars[k]) matching = true;
        if (!matching)
        {
          retstr[OPtr] = ActChar; OPtr++;
        } else {
          retstr[OPtr] = replace; OPtr++;
        }
      }
    }
    prev_char = ActChar;
  }
  retstr[OPtr] = 0; OPtr++;
  return retstr;
}

gchar* cm_string_interpret_first_string_old(const gchar *inp, gint *RetPtr)
{
  gboolean finished = false;
  gboolean in_string = false;
  guint reinterpret_numeric_phaze = 0;
  gint OPtr=0;
  gchar prev_char = ' ';
  gint strlen_inp = strlen(inp);
  gchar *retstr = (gchar *) g_new (gchar, strlen_inp+1);
  if (RetPtr) *RetPtr = 0;
  for (gint APtr=0;(APtr<strlen_inp)&&(!finished); APtr++)
  {
      gchar ActChar = inp[APtr];
      if (in_string)
      {
	  if ((ActChar=='\"') && (prev_char != '\\'))
	  {
	      in_string = false;
	      finished = true;
	      retstr[OPtr] = 0;
	      reinterpret_numeric_phaze = 0;
	      if (RetPtr)
	      {
		  *RetPtr = APtr;
	      }
	  }
	  else
	  {
	      bool reinterpreted = false;
	      if (!((ActChar >= '0')&&(ActChar <= '9'))) reinterpret_numeric_phaze = 0;
	      if ((reinterpret_numeric_phaze)&&((ActChar >= '0')&&(ActChar <= '9')))
	      {
		  reinterpret_numeric_phaze++;
		  if ((reinterpret_numeric_phaze==3) && (OPtr >= 3))
		  {
		      retstr[OPtr] = ActChar;
		      retstr[OPtr+1]=0;
		      int char_val = atoi(retstr+(OPtr-2));
		      if ((char_val>=0) && (char_val<=255))
		      {
			  OPtr-=2;
			  retstr[OPtr-1] = (gchar)((guint)char_val);
			  reinterpreted = true;
		      }
		  }
	      }
	      if ((prev_char == '\\') && (OPtr != 0))
	      {
		  if (ActChar=='\"') { retstr[OPtr-1] = '\"'; reinterpreted = true; }
		  if (ActChar== 'r') { retstr[OPtr-1] = '\r'; reinterpreted = true; }
		  if (ActChar== 'n') { retstr[OPtr-1] = '\n'; reinterpreted = true; }
		  if (ActChar== 't') { retstr[OPtr-1] = '\t'; reinterpreted = true; }
		  if ((ActChar >= '0')&&(ActChar <= '9')) reinterpret_numeric_phaze = 1;
	      }
	      if (!reinterpreted)
	      {
		  retstr[OPtr] = ActChar; OPtr++;
	      }
	  }
      } else {
	  reinterpret_numeric_phaze = 0;
	  if (ActChar=='\"')
	  {
	      in_string = true;
	  }
      }
      prev_char = ActChar;
  }
  if (finished)
  {
    return retstr;
  } else {
    g_free(retstr);
    return NULL;
  }
}



void cm_string_statistics(const gchar *inp,
                          gint *_Containing_strings,  bool *_Containing_noncomplete_string, bool *_Strings_have_escape,
                          gint *_BlankChars_before, gint *_BlankChars_middle, gint *_BlankChars_after,
                          gint *_NonBlankChars_before, gint *_NonBlankChars_middle, gint *_NonBlankChars_after,
                          const gchar *blank)
{
  gint Containing_strings = 0;
  bool Containing_noncomplete_string = false;
  bool Strings_have_escape = false;
  gint BlankChars_before = 0;
  gint BlankChars_middle = 0;
  gint BlankChars_after  = 0;
  gint NonBlankChars_before = 0;
  gint NonBlankChars_middle = 0;
  gint NonBlankChars_after  = 0;

  gboolean in_string = false;
  gchar prev_char = ' ';
  gint strlen_inp = strlen(inp);
  for (gint APtr=0;APtr<strlen_inp; APtr++)
  {
    gchar ActChar = inp[APtr];
    if (in_string)
    {
      if ((ActChar=='\"') && (prev_char != '\\'))
      {
        Containing_strings ++;
        in_string = false;
      } else {
        if (prev_char == '\\')
        {
          Strings_have_escape = true;
        }
      }
    } else {
      if (ActChar=='\"')
      {
        in_string = true;
        if (Containing_strings)
        {
          BlankChars_middle = BlankChars_after;
          NonBlankChars_middle = NonBlankChars_after;
          BlankChars_after = 0;
          NonBlankChars_after = 0;
        }
      } else {
        bool isBlank = false;
        for (unsigned j=0; (j<strlen(blank)) && (!isBlank); j++) if (ActChar == blank[j]) isBlank = true;
        if (Containing_strings)
        {
          if (isBlank) BlankChars_after++; else NonBlankChars_after++;
        } else {
          if (isBlank) BlankChars_before++; else NonBlankChars_before++;
        }
      }
    }
    prev_char = ActChar;
  }
  if (in_string) Containing_noncomplete_string = true;

  if (_Containing_strings != NULL) *_Containing_strings = Containing_strings;
  if (_Containing_noncomplete_string != NULL) *_Containing_noncomplete_string = Containing_noncomplete_string;
  if (_Strings_have_escape != NULL) *_Strings_have_escape = Strings_have_escape;
  if (_BlankChars_before != NULL) *_BlankChars_before = BlankChars_before;
  if (_BlankChars_middle != NULL) *_BlankChars_middle = BlankChars_middle;
  if (_BlankChars_after != NULL) *_BlankChars_after = BlankChars_after;
  if (_NonBlankChars_before != NULL) *_NonBlankChars_before = NonBlankChars_before;
  if (_NonBlankChars_middle != NULL) *_NonBlankChars_middle = NonBlankChars_middle;
  if (_NonBlankChars_after != NULL) *_NonBlankChars_after = NonBlankChars_after;
}

bool cm_string_is_first_nonblank_char_from_defined(const gchar *inp, const gchar *defined_chars, const gchar *blank)
{
  for (unsigned i=0; i<strlen(inp); i++)
  {
    bool isBlank = false;
    for (unsigned j=0; (j<strlen(blank)) && (!isBlank); j++) if (inp[i] == blank[j]) isBlank = true;
    if (!isBlank)
    {
      bool isDefined = false;
      for (unsigned j=0; (j<strlen(defined_chars)) && (!isDefined); j++) if (inp[i] == defined_chars[j]) isDefined = true;
      if (isDefined) return true; else return false;
    }
  }
  return false;
}

bool cm_string_is_last_nonblank_char_from_defined(const gchar *inp, const gchar *defined_chars, const gchar *blank)
{
  for (unsigned ii=strlen(inp); ii>0; ii--)
  {
    unsigned i = ii-1;
    bool isBlank = false;
    for (unsigned j=0; (j<strlen(blank)) && (!isBlank); j++) if (inp[i] == blank[j]) isBlank = true;
    if (!isBlank)
    {
      bool isDefined = false;
      for (unsigned j=0; (j<strlen(defined_chars)) && (!isDefined); j++) if (inp[i] == defined_chars[j]) isDefined = true;
      if (isDefined) return true; else return false;
    }
  }
  return false;
}


gchar cm_string_get_first_char_except(const gchar *inp, const gchar *blank)
{
  gint len = strlen(inp);
  gint lenblank = strlen(blank);
  for (gint i=0; i<len; i++)
  {
    bool found = false;
    for (gint j=0; (j<lenblank) && (!found); j++)
    {
      if (inp[i] == blank[j]) found = true;
    }
    if (!found) return inp[i];
  }
  return 0;
}


gchar *cm_string_remove_first_pair_quotes(const gchar *inp, const gchar quote_begin, const gchar quote_end, const gchar *blank)
{
  gint pos1 = -1;
  gint pos2 = -1;
  gint len = strlen(inp);
  gint lenblank = strlen(blank);
  if (len<1) return NULL;
  for (gint i=0; (i<len) && (pos1==-1); i++)
  {
    if (inp[i] == quote_begin)
    {
      pos1 = i;
    } else {
      bool found = false;
      for (gint j=0; (j<lenblank) && (!found); j++)
      {
        if (inp[i] == blank[j]) found = true;
      }
      if (!found) return NULL;
    }
  }
  for (gint i=len-1; (i>=0) && (pos2==-1); i--)
  {
    if (inp[i] == quote_end  )
    {
      pos2 = i;
    } else {
      bool found = false;
      for (gint j=0; (j<lenblank) && (!found); j++)
      {
        if (inp[i] == blank[j]) found = true;
      }
      if (!found) return NULL;
    }
  }
  if ((pos1 != -1)&& (pos2 != -1) && ((pos1+1) < pos2))
  {
   return g_strndup (inp+pos1+1, pos2-(pos1+1));
  }
  return NULL;
}

gchar *cm_string_remove_first_brackets_pair_or_interpret_string(const gchar *inp, const gchar *brackets, const gchar *blank)
{
  gchar *RetVal = NULL;
  gint lenbrackets = strlen(brackets);
  gchar first_char = cm_string_get_first_char_except(inp, blank);
  
  if (first_char=='\"')
  {
    RetVal = cm_string_interpret_first_string(inp);
  } else {
    for (gint i=0; i<(lenbrackets/2) && (RetVal==NULL); i++)
    {
      if (first_char==brackets[(i*2)+0])
      {
        RetVal = cm_string_remove_first_pair_quotes(inp,brackets[(i*2)+0], brackets[(i*2)+1], blank);
      }
    }
  }
  if (RetVal == NULL) RetVal = g_strdup (inp);
  return RetVal;
}



int cm_string_get_value_int_bool_helper(const gchar *str)
{
  if (!strcasecmp(str,"Y")) return 1;
  if (!strcasecmp(str,"YES")) return 1;
  if (!strcasecmp(str,"ANO")) return 1;
  if (!strcasecmp(str,"TRUE")) return 1;
  if (!strcasecmp(str,"T")) return 1;
  if (!strcasecmp(str,"N")) return 0;
  if (!strcasecmp(str,"NO")) return 0;
  if (!strcasecmp(str,"NE")) return 0;
  if (!strcasecmp(str,"ON")) return 1;
  if (!strcasecmp(str,"OFF")) return 0;
  if (!strcasecmp(str,"FALSE")) return 0;
  if (!strcasecmp(str,"F")) return 0;
  return 2;
}

// template <typename T>
// T cm_string_get_value_templ(const gchar *str)
// {
//     if (str[0] == '\0')
// 	return 0;
//     int ret = cm_string_get_value_int_bool_helper(str);
//     if (ret != 2)
// 	return (int)ret;
//     std::stringstream ss(str);
//     ss >> ret;
//     if (!ss)
//     {
// 	printf("cm_string_get_value_templ: from '%s'\n", str);
// 	abort();
//     }
//     return ret;
// }

// int cm_string_get_value_int(const gchar *str) { return cm_string_get_value_templ<int>(str); }
// uint64_t cm_string_get_value_uint64(const gchar *str) { return cm_string_get_value_templ<uint64_t>(str); }
// int64_t cm_string_get_value_int64(const gchar *str) { return cm_string_get_value_templ<int64_t>(str); }
// double cm_string_get_value_double(const gchar *str) { return cm_string_get_value_templ<double>(str); }

int cm_string_get_value_int(const gchar *str)
{
  int retval = 0;
  int type=0;

  int ret = cm_string_get_value_int_bool_helper(str);
  if (ret != 2) return (int)ret;

  if (strlen(str)>2)
  {
    if ((str[0]=='0') && ((str[1]=='X') || (str[1]=='x'))) type=1;
  }
  if (type==0)
  {
    retval = atoi(str);
  } else if (type==1)
  {
    char *ptr;
    retval = strtol(str+2,&ptr,16);
  }
  return retval;
}



uint64_t cm_string_get_value_uint64(const gchar *str)
{
  uint64_t retval = 0;
  int type=0;

  int ret = cm_string_get_value_int_bool_helper(str);
  if (ret != 2) return (uint64_t)ret;


  if (strlen(str)>2)
  {
    if ((str[0]=='0') && ((str[1]=='X') || (str[1]=='x'))) type=1;
  }
  if (type==0)
  {
    char *ptr;
    retval = strtoull(str+0,&ptr,10);
  } else if (type==1)
  {
    char *ptr;
    retval = strtoull(str+2,&ptr,16);
  }
  return retval;
}

int64_t cm_string_get_value_int64(const gchar *str)
{
  uint64_t retval = 0;
  int type=0;

  int ret = cm_string_get_value_int_bool_helper(str);
  if (ret != 2) return (int64_t)ret;


  if (strlen(str)>2)
  {
    if ((str[0]=='0') && ((str[1]=='X') || (str[1]=='x'))) type=1;
  }
  if (type==0)
  {
    char *ptr;
    retval = strtoll(str+0,&ptr,10);
  } else if (type==1)
  {
    char *ptr;
    retval = strtoll(str+2,&ptr,16);
  }
  return retval;
}

double cm_string_get_value_double(const gchar *str)
{
  char *ptr;
  return g_ascii_strtod(str,&ptr);
}

bool cm_string_get_value_bool(const gchar *str) { return cm_string_get_value_int(str); }

bool cm_string_is_there_minus(const gchar *str)
{
  for (unsigned i=0; i<strlen(str); i++)
  {
    if (str[i] == '-') return true;
  }
  return false;
}






// void cm_string_TESTER(void)
// {
//   char *teststr;
//   asprintf(&teststr,"%s", ";;;;aaa[1;2;3];; bbb(1;2;3)\";\\n\\n\\n\\n \\t \\\" ; \n ; \";\"xx\\010yy\\nzz\"aa\"");
//  debug_printf("TESTSTR=%s\n-------------",teststr);
//   gchar **item = cm_string_strsplit_outside_string_and_brackets(teststr, ";", "[]()", 0);
//   if (item)
//   {
//     int i=0;
//     while (item[i])
//     {
//      debug_printf("-->>Item[%d]=\"%s\"",i,item[i]);
// //      if ((i==1)||(i==2))
//       {
//         gchar* interpreted = cm_string_interpret_first_string(item[i]);
//         if (interpreted)
//         {
//           if (interpreted[0])
//           {
//            debug_printf("-->>Interpreted[%d]=\"%s\"",i,interpreted);
//             gchar* escaped = cm_string_escape_string(interpreted, "#\n\"\r\t");
//            debug_printf("-->>Escaped[%d]=\"%s\"",i,escaped);
//             g_free(escaped);
//           }
//           g_free(interpreted);
//         }
//       }
//       i++;
//     }
//     g_strfreev(item);
//   }
//  debug_printf("-----------------");
//   free(teststr);
// }

//uint64_t cm_string_get_value_uint64(const gchar *str)
//int64_t cm_string_get_value_int64(const gchar *str)



std::string escape_special_chars_and_advance(WeakString &input, size_t limit)
{
    std::string output;
    output.reserve(input.size());
    while (!input.empty())
    {
	char c = input.front();
	if (c == '\\')
	{
	    if (output.size() + 2 >= limit)
		break;
	    output += "\\\\";
	}
	else if (c == '"')
	{
	    if (output.size() + 2 >= limit)
		break;
	    output += "\\\"";
	}
	else if (c >= ' ')
	{
	    // Printable
	    if (output.size() + 1 >= limit)
		break;
	    output.push_back(c);
	}
	else
	{
	    // Non-printable
	    if (output.size() + 2 >= limit)
		break;

	    if (c == '\n')
		output += "\\n";
	    else if (c == '\t')
		output += "\\t";
	    else
	    {
		if (output.size() + 4 >= limit)
		    break;
		char buffer[10];
		sprintf(buffer, "\\%03u", (unsigned)(unsigned char)c);
		output += std::string(buffer, buffer + 4);
	    }
	}
	input.advance(1);
    }
    return output;
}

std::string escape_special_chars(WeakString input)
{
    return escape_special_chars_and_advance(input, SIZE_MAX);
}

std::string unescape_special_chars(WeakString str, char delimiter)
{
    WeakStringInput input(str);
    return unescape_special_chars_and_advance(input, delimiter);
}

template <typename Input>
std::string unescape_special_chars_and_advance_templ(Input &input, char delimiter)
{
    std::string output;
    while (input.request(1) > 0)
    {
	if (input[0] == delimiter)
	{
	    input.advance(1);
	    return output;
	}
	if (input[0] == '\\')
	{
	    input.advance(1);
	    CM_CHECK(input.request(1) > 0, "Expected character after escape char");
	    if (input[0] == '\\') { output.push_back('\\'); }
	    else if (input[0] == '"') { output.push_back('"'); }
	    else if (input[0] == 'n') { output.push_back('\n'); }
	    else if (input[0] == 't') { output.push_back('\t'); }
	    else if (isdigit(input[0]))
	    {
		CM_CHECK(input.request(3) >= 3 && isdigit(input[1]) && isdigit(input[2]), "Expected three digits after escape char and digit");
		int code = (input[0] - '0') * 100 + (input[1] - '0') * 10 + (input[2] - '0');
		CM_CHECK(code <= 255, "Numeric escape sequence too big: %d", code);
		output.push_back(code);
		input.advance(2);
	    }
	    else
	    {
		CM_THROW("Invalid escape character %d", (int)input[0]);
	    }
	}
	else
	{
	    output.push_back(input[0]);
	}
	input.advance(1);
    }
    CM_THROW("unterminated escaped string");
}

std::string unescape_special_chars_and_advance(WeakStringInput &input, char delimiter)
{
    return unescape_special_chars_and_advance_templ(input, delimiter);
}

std::string unescape_special_chars_and_advance(WeakString &input, char delimiter)
{
    WeakStringInput in(input);
    std::string result = unescape_special_chars_and_advance_templ(in, delimiter);
    input = in.input; // posune vstup
    return result;
}

std::string unescape_special_chars_and_advance(IOStreamInput &input, char delimiter)
{
    return unescape_special_chars_and_advance_templ(input, delimiter);
}

std::string interpret_first_string_and_advance(WeakString &input)
{
    while (!input.empty() && input.front() != '\"')
	input.advance(1);
    CM_CHECK(!input.empty() && input.front() == '\"', "Expected beginning of a string in %s", input.str().c_str());
    input.advance(1);
    WeakStringInput in(input);
    std::string output = unescape_special_chars_and_advance(in, '\"');
    input = in.input; // posune vstup
    return output;
}

gchar *cm_string_escape_string(WeakString input)
{
    try
    {
	std::string s = "\"" + escape_special_chars(input) + "\"";
	return g_strdup(s.c_str());
    }
    catch (std::runtime_error &e)
    {
	cms_ns_if_print("libcm", 1, "cm_string_escape_string error: %s, in %s", e.what(), input.str().c_str());
	return 0;
    }
}

gchar* cm_string_interpret_first_string_new(const gchar *inp, gint *RetPtr)
{
    try
    {
	// cms_ns_if_print("libcm", 1, "cm_string_intepret_first_string: %s", inp);
	WeakString input(inp);
	if (RetPtr)
	    *RetPtr = 0;
	std::string s = interpret_first_string_and_advance(input);
	if (RetPtr)
	    *RetPtr = input.begin() - inp - 1;
	return g_strdup(s.c_str());
    }
    catch (std::runtime_error &e)
    {
	// cms_ns_if_print("libcm", 1, "cm_string_intepret_first_string error: %s", e.what());
	return 0;
    }
}

gchar* cm_string_interpret_first_string(const gchar *inp, gint *RetPtr)
{
    return cm_string_interpret_first_string_new(inp, RetPtr);
    // gint old_ret, new_ret;
    // gchar *old_result = cm_string_interpret_first_string_old(inp, &old_ret);
    // gchar *new_result = cm_string_interpret_first_string_new(inp, &new_ret);
    // cms_ns_if_print("libcm", 1, "cm_string_intepret_first_string old: >%s<: %d: >%s<", inp, old_ret, old_result);
    // cms_ns_if_print("libcm", 1, "cm_string_intepret_first_string new: >%s<: %d: >%s<", inp, new_ret, new_result);
    // assert(old_ret == new_ret);
    // assert(old_result == new_result || (old_result != NULL && new_result != NULL && strcmp(old_result, new_result) == 0));
    // if (RetPtr)
    // 	*RetPtr = new_ret;
    // return old_result;
}

