
#include "cm_std_string.hh"



std::string cm_std_string_InterpretStringIfIsFullyInQuotes(std::string Inp, bool EraseBlanksOutsideString)
{
//  if (cm_string_is_first_nonblank_char_from_defined(Inp.c_str(), "\"") && cm_string_is_last_nonblank_char_from_defined(Inp.c_str(), "\""))// tohle je kravina
//  TOTO NEFUNGUJE DOBRE PROTOZE "1,2"="exist(Sim_A__SW_M1)" NOVA GENIALNI FUNKCE PREVEDE NA 1,2

  gint Containing_strings;
  bool Containing_noncomplete_string;
  bool Strings_have_escape;
  gint BlankChars_before;
  gint BlankChars_middle;
  gint BlankChars_after;
  gint NonBlankChars_before;
  gint NonBlankChars_middle;
  gint NonBlankChars_after;

  cm_std_string_statistics(Inp, &Containing_strings, &Containing_noncomplete_string, &Strings_have_escape,
                           &BlankChars_before, &BlankChars_middle, &BlankChars_after,
                           &NonBlankChars_before, &NonBlankChars_middle, &NonBlankChars_after, " \t\n\r" );


  if ((Containing_strings == 1) && (!Containing_noncomplete_string) && (NonBlankChars_before==0) && (NonBlankChars_middle==0) && (NonBlankChars_after==0))
  {
    gchar *inside = cm_string_interpret_first_string(Inp.c_str());
    if (inside)
    {
      std::string RetVal = std::string(inside);
      g_free(inside);
      return RetVal;
    }
    return Inp;
  } else {
    if (EraseBlanksOutsideString)
    {
      char *line_no_space = cm_string_strdup_nospace_outside_string(Inp.c_str());
      if (line_no_space != NULL)
      {
        std::string RetVal = std::string(line_no_space);
        g_free(line_no_space);
        return RetVal;
      }
    }
    return Inp;
  }
}

template <typename Out>
void cm_std_string_SplitLineAndInterpretStringsInQuotesTempl(std::string Inp,
                                                        Out &OutParams,
                                                        const gchar *delimiter,
                                                        const gchar *brackets,
                                                        bool EraseBlanksOutsideString,
                                                        bool debug_me,
                                                        bool interpret_str_enabled)
{
  char *line_no_space = NULL;
//  if (EraseBlanksOutsideString) line_no_space = cm_string_strdup_nospace_outside_string(Inp.c_str());
  char **item = cm_string_strsplit_outside_string_and_brackets(((line_no_space == NULL) ? Inp.c_str() : line_no_space), delimiter, brackets, 0, debug_me);
  if (item)
  {
    int i=0;
    while ((item[i]))
    {
      if (interpret_str_enabled)
      {
        OutParams.push_back(cm_std_string_InterpretStringIfIsFullyInQuotes(cm_std_string_trim(std::string(item[i]))));
//        OutParams.push_back(cm_std_string_InterpretStringIfIsFullyInQuotes(std::string(item[i])));
      } else {
        OutParams.push_back(cm_std_string_trim(std::string(item[i])));
//        OutParams.push_back(std::string(item[i]));
      }
      i++;
    }
    g_strfreev(item);
  }
  if (line_no_space != NULL) g_free(line_no_space);
}

void cm_std_string_SplitLineAndInterpretStringsInQuotes(std::string Inp,
                                                        std::deque<std::string> &OutParams,
                                                        const gchar *delimiter,
                                                        const gchar *brackets,
                                                        bool EraseBlanksOutsideString,
                                                        bool debug_me,
                                                        bool interpret_str_enabled)
{
    cm_std_string_SplitLineAndInterpretStringsInQuotesTempl(Inp, OutParams, delimiter, brackets,
                                                            EraseBlanksOutsideString, debug_me,
                                                            interpret_str_enabled);
}

void cm_std_string_SplitLineAndInterpretStringsInQuotes(std::string Inp,
                                                        StringList &OutParams,
                                                        const gchar *delimiter,
                                                        const gchar *brackets,
                                                        bool EraseBlanksOutsideString,
                                                        bool debug_me,
                                                        bool interpret_str_enabled)
{
    cm_std_string_SplitLineAndInterpretStringsInQuotesTempl(Inp, OutParams, delimiter, brackets,
                                                            EraseBlanksOutsideString, debug_me,
                                                            interpret_str_enabled);
}

int      cm_std_string_get_value_int(std::string str) { return cm_string_get_value_int(str.c_str()); }
uint64_t cm_std_string_get_value_uint64(std::string str) { return cm_string_get_value_uint64(str.c_str()); }
int64_t  cm_std_string_get_value_int64(std::string str) { return cm_string_get_value_int64(str.c_str()); }
double   cm_std_string_get_value_double(std::string str) { return cm_string_get_value_double(str.c_str()); }
bool     cm_std_string_get_value_bool(std::string str) { return cm_string_get_value_bool(str.c_str()); }

void cm_std_string_statistics(std::string inp,
                              gint *_Containing_strings,  bool *_Containing_noncomplete_string, bool *_Strings_have_escape,
                              gint *_BlankChars_before, gint *_BlankChars_middle, gint *_BlankChars_after,
                              gint *_NonBlankChars_before, gint *_NonBlankChars_middle, gint *_NonBlankChars_after,
                              const gchar *blank)
{
  cm_string_statistics(inp.c_str(), _Containing_strings,  _Containing_noncomplete_string, _Strings_have_escape, _BlankChars_before, _BlankChars_middle, _BlankChars_after,
                        _NonBlankChars_before, _NonBlankChars_middle, _NonBlankChars_after, blank);
}

void cm_std_string_replace(std::string& subject, const std::string& search, const std::string& replace)
{
  size_t pos = 0;
  while((pos = subject.find(search, pos)) != std::string::npos)
  {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

void cm_std_string_insert(std::string& subject, size_t pos, const std::string& insert)
{
  subject.insert(pos, insert);
}

void cm_std_string_replace_by_map(std::string& subject, std::map<std::string, std::string> &rmap)
{
  for (std::map<std::string, std::string>::iterator it = rmap.begin(); it != rmap.end(); it++)
  {
    cm_std_string_replace(subject, it->first, it->second);
  }
}

void cm_std_string_g_strsplit(std::string Inp, std::string Delimiter, std::list<std::string> &Output)
{
  char **item = g_strsplit(Inp.c_str(), Delimiter.c_str(), -1);
  if (item)
  {
    unsigned i = 0;
    while(item[i])
    {
      Output.push_back(std::string(item[i]));
      i++;
    }
    g_strfreev(item);
  }
}



gchar cm_std_string_get_first_char_except(std::string inp, const gchar *blank)
{
  return cm_string_get_first_char_except(inp.c_str(), blank);
}

std::string cm_std_string_remove_first_pair_quotes(std::string inp, const gchar quote_begin, const gchar quote_end, const gchar *blank)
{
  std::string RetVal;
  gchar *ret = cm_string_remove_first_pair_quotes(inp.c_str(), quote_begin, quote_end, blank);
  if (ret != NULL)
  {
    RetVal = std::string(ret);
    g_free(ret);
  }
  return RetVal;
}

std::string cm_std_string_remove_first_brackets_pair_or_interpret_string(std::string inp, const gchar *brackets, const gchar *blank)
{
  std::string RetVal;
  gchar *ret = cm_string_remove_first_brackets_pair_or_interpret_string(inp.c_str(), brackets, blank);
  if (ret != NULL)
  {
    RetVal = std::string(ret);
    g_free(ret);
  }
  return RetVal;
}


// trim from end of string (right)
std::string cm_std_string_rtrim(std::string s, const gchar* t)
{
  s.erase(s.find_last_not_of(t) + 1);
  return s;
} 

// trim from beginning of string (left)
std::string cm_std_string_ltrim(std::string s, const gchar* t)
{
  s.erase(0, s.find_first_not_of(t));
  return s;
}
  
// trim from both ends of string (left & right)
std::string cm_std_string_trim(std::string s, const gchar* t)
{
    return cm_std_string_ltrim(cm_std_string_rtrim(s));
}

template <typename T>
std::string cm_std_string_print_short_templ(T f, int max_decimals, char const *fmt1)
{
    std::string RetVal = "0";
    char *wstr = NULL;
    if (max_decimals < 0)
	max_decimals = 10;
    asprintf(&wstr,fmt1, max_decimals, f);
    if (wstr != NULL)
    {
	RetVal = wstr;
	free(wstr);
    }
    if (max_decimals > 0) 
    {
	// Vysledek obsahuje desetinnou carku, je tvaru [-]ddd.ddd0000
	while (!RetVal.empty() && RetVal[RetVal.size() - 1] == '0')
	    RetVal.erase(RetVal.end() - 1);
	if (!RetVal.empty() && RetVal[RetVal.size() - 1] == '.')
            RetVal.erase(RetVal.end() - 1);
    }
    if (RetVal == "-0")
	RetVal = "0";
    
    // cm_std_string_rtrim(RetVal, "0");
    // RetVal = cm_std_string_ltrim(RetVal, "0");
    // RetVal = cm_std_string_rtrim(cm_std_string_rtrim(RetVal, "0"), ".");
    // if (RetVal.size()==0) RetVal = "0";
    return RetVal;
}

std::string cm_std_string_print_short_float(float f, int max_decimals)
{
    return cm_std_string_print_short_templ(f, max_decimals, "%.*f");
}

std::string cm_std_string_print_short_float(double f, int max_decimals)
{
    return cm_std_string_print_short_templ(f, max_decimals, "%.*f");
}

std::string cm_std_string_print_short_float(long double f, int max_decimals)
{
    return cm_std_string_print_short_templ(f, max_decimals, "%.*Lf");
}

// std::string cm_std_string_print_short_float(float f, int max_decimals)
// {
//   std::string RetVal = "0";
//   char *wstr = NULL;
//   if (max_decimals < 0)
//   {
//     asprintf(&wstr,"%f",f);
//   } else {
//     char fmt[20];
//     sprintf(fmt,"%c.%df",'%',max_decimals);
//     asprintf(&wstr,fmt,f);
//   }
//   if (wstr != NULL)
//   {
//     RetVal = std::string(wstr);
//     free(wstr);
//   }
//   RetVal = cm_std_string_ltrim(RetVal, "0");
//   RetVal = cm_std_string_rtrim(cm_std_string_rtrim(RetVal, "0"), ".");
//   if (RetVal.size()==0) RetVal = "0";
//   return RetVal;
// }

// std::string cm_std_string_print_short_float(double f, int max_decimals)
// {
//     std::string RetVal = "0";
//     char *wstr = NULL;
//     if (max_decimals < 0)
//     {
// 	asprintf(&wstr,"%f",f); // Pozn: Default je 6
//     }
//     else
//     {
// 	char fmt[20];
// 	sprintf(fmt,"%c.%df",'%',max_decimals);
// 	asprintf(&wstr,fmt,f);
//     }
//     if (wstr != NULL)
//     {
// 	RetVal = std::string(wstr);
// 	free(wstr);
//     }
//     RetVal = cm_std_string_ltrim(RetVal, "0");
//     RetVal = cm_std_string_rtrim(cm_std_string_rtrim(RetVal, "0"), ".");
//     if ((RetVal.size()==0) || ((RetVal.size()>0) && (RetVal.c_str()[0]=='.')))
// 	RetVal = "0" + RetVal;
//     return RetVal;
// }

// std::string cm_std_string_print_short_float(long double f, int max_decimals)
// {
//   std::string RetVal = "0";
//   char *wstr = NULL;
//   if (max_decimals < 0)
//   {
//     asprintf(&wstr,"%Lf",f);
//   } else {
//     char fmt[20];
//     sprintf(fmt,"%c.%dLf",'%',max_decimals);
//     asprintf(&wstr,fmt,f);
//   }
//   if (wstr != NULL)
//   {
//     RetVal = std::string(wstr);
//     free(wstr);
//   }
//   RetVal = cm_std_string_ltrim(RetVal, "0");
//   RetVal = cm_std_string_rtrim(cm_std_string_rtrim(RetVal, "0"), ".");
//   if (RetVal.size()==0) RetVal = "0";
//   return RetVal;
// }
