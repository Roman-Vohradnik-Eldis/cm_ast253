
#ifndef CM_STD_STRING_H
#define CM_STD_STRING_H

#include <deque>
#include <list>
#include <map>
#include "cm_string.hh"
#include "cm_types.hh"

void cm_std_string_statistics(std::string inp,
                              gint *_Containing_strings,  bool *_Containing_noncomplete_string, bool *_Strings_have_escape,
                              gint *_BlankChars_before, gint *_BlankChars_middle, gint *_BlankChars_after,
                              gint *_NonBlankChars_before, gint *_NonBlankChars_middle, gint *_NonBlankChars_after,
                              const gchar *blank=" \t\n\r\f\v" );

std::string cm_std_string_InterpretStringIfIsFullyInQuotes(std::string Inp, bool EraseBlanksOutsideString = false);

void        cm_std_string_SplitLineAndInterpretStringsInQuotes(std::string Inp,
                                                               StringList &OutParams,
                                                               const gchar *delimiter,
                                                               const gchar *brackets,
                                                               bool EraseBlanksOutsideString = false,
                                                               bool debug_me = false,
                                                               bool interpret_str_enabled=true);

void        cm_std_string_SplitLineAndInterpretStringsInQuotes(std::string Inp,
                                                               std::deque<std::string> &OutParams,
                                                               const gchar *delimiter,
                                                               const gchar *brackets,
                                                               bool EraseBlanksOutsideString = false,
                                                               bool debug_me = false,
                                                               bool interpret_str_enabled=true);


void        cm_std_string_g_strsplit(std::string Inp, std::string Delimiter, std::list<std::string> &Output);

int         cm_std_string_get_value_int(std::string str);
uint64_t    cm_std_string_get_value_uint64(std::string str);
int64_t     cm_std_string_get_value_int64(std::string str);
double      cm_std_string_get_value_double(std::string str);
bool        cm_std_string_get_value_bool(std::string str);
void        cm_std_string_replace(std::string& subject, const std::string& search, const std::string& replace);
void        cm_std_string_replace_by_map(std::string& subject, std::map<std::string, std::string> &rmap);
void 		cm_std_string_insert(std::string& subject, size_t pos, const std::string& insert);
std::string cm_std_string_rtrim  (std::string s, const gchar* t = " \t\n\r\f\v");
std::string cm_std_string_ltrim  (std::string s, const gchar* t = " \t\n\r\f\v");
std::string cm_std_string_trim   (std::string s, const gchar* t = " \t\n\r\f\v");

// Je-li max_decimals == -1, pouzije se 10
std::string cm_std_string_print_short_float(float       f, int max_decimals = -1);
std::string cm_std_string_print_short_float(double      f, int max_decimals = -1);
std::string cm_std_string_print_short_float(long double f, int max_decimals = -1);


gchar cm_std_string_get_first_char_except(std::string inp, const gchar *blank = " \t\n\r\f\v");
std::string cm_std_string_remove_first_pair_quotes(std::string inp, const gchar quote_begin='(', const gchar quote_end=')', const gchar *blank = " \t\n\r\f\v");
std::string cm_std_string_remove_first_brackets_pair_or_interpret_string(std::string inp, const gchar *brackets="()[]{}", const gchar *blank = " \t\n\r\f\v");


#endif // CM_STD_STRING_H
