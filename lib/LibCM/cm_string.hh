#ifndef CM_STRING_H
#define CM_STRING_H

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <string>
#include "weak_string.hh"

std::string cm_string_format(char const *fmt, ...);

#define CM_CHECK(expr, fmt, ...) do { if (!(expr)) throw std::runtime_error(std::string(#expr) + cm_string_format(fmt, ##__VA_ARGS__)); } while (0)
#define CM_THROW(fmt, ...) throw std::runtime_error(cm_string_format(fmt, ##__VA_ARGS__))


gchar *  cm_string_strdup_nospace_outside_string(const gchar *inp, const gchar *delchars = " \t\n\r\f\v");
gchar *  cm_string_strdup_replace_outside_string(const gchar *inp, const gchar *delchars = " \t\n\r\f\v", gchar replace = ' ');

gchar *  cm_string_interpret_first_string(const gchar *inp, gint *RetPtr = NULL);


// gchar *  cm_string_escape_string(const gchar *inp, const gchar *characters = "#\"\t\n\r\f\v\\\\", bool add_quotes = true);

// gchar *  cm_string_escape_string_with_limit(const gchar *inp, size_t max_size, const gchar *characters = "#\"\t\n\r\f\v\\\\", bool add_quotes = true);

// std::string cm_string_escape_string_with_limit(WeakString input, size_t max_size);

bool     cm_string_is_first_nonblank_char_from_defined(const gchar *inp, const gchar *defined_chars, const gchar *blank=" \t\n\r\f\v");
bool     cm_string_is_last_nonblank_char_from_defined(const gchar *inp, const gchar *defined_chars, const gchar *blank=" \t\n\r\f\v");

gchar**  cm_string_strsplit_outside_string(const gchar *inp, const gchar *delimiter, gint  max_tokens);

// Rozseka string podle delimiteru, ktery muze byt viceznakovy. Pritom:
//
// To, co je v uvozovkach "", tak neprozkoumava. Uvnitr uvozovek jdou
// escapovat uvozovky pomoci zpetneho lomitka.
//
// V zavorkach delimitery neprozkoumava
//
// max_tokens se nyni ignoruje.
//
gchar**  cm_string_strsplit_outside_string_and_brackets(const gchar *inp, const gchar *delimiter, const gchar *brackets, gint  max_tokens, bool debug_me = false);

bool     cm_string_is_there_minus(const gchar *str);
int      cm_string_get_value_int(const gchar *str);
uint64_t cm_string_get_value_uint64(const gchar *str);
int64_t  cm_string_get_value_int64(const gchar *str);
double   cm_string_get_value_double(const gchar *str);
bool     cm_string_get_value_bool(const gchar *str);


void cm_string_statistics(const gchar *inp,
                          gint *_Containing_strings,  bool *_Containing_noncomplete_string, bool *_Strings_have_escape,
                          gint *_BlankChars_before, gint *_BlankChars_middle, gint *_BlankChars_after,
                          gint *_NonBlankChars_before, gint *_NonBlankChars_middle, gint *_NonBlankChars_after,
                          const gchar *blank=" \t\n\r\f\v"
                         );


gchar cm_string_get_first_char_except(const gchar *inp, const gchar *blank = " \t\n\r\f\v");
gchar *cm_string_remove_first_pair_quotes(const gchar *inp, const gchar quote_begin='(', const gchar quote_end=')', const gchar *blank = " \t\n\r\f\v");
gchar *cm_string_remove_first_brackets_pair_or_interpret_string(const gchar *inp, const gchar *brackets="()[]{}", const gchar *blank = " \t\n\r\f\v");


// void cm_string_TESTER(void);

#ifndef __PRI64_PREFIX
# if __WORDSIZE == 64
#  define __PRI64_PREFIX "l"
# else
#  define __PRI64_PREFIX "ll"
# endif
#endif // __PRI64_PREFIX

#ifndef PRId64
# define PRId64 __PRI64_PREFIX "d"
#endif

#ifndef PRIu64
# define PRIu64 __PRI64_PREFIX "u"
#endif

#ifndef PRIx64
# define PRIx64 __PRI64_PREFIX "x"
#endif

#ifndef PRIX64
# define PRIX64 __PRI64_PREFIX "X"
#endif

#ifndef PRIo64
# define PRIo64 __PRI64_PREFIX "o"
#endif


std::string escape_special_chars_and_advance(WeakString &input, size_t limit);
std::string escape_special_chars(WeakString input);

// Zere vstupni string a interpretuje escapovaci sekvence znaky, dokud
// nenarazi na delimiter.
// 
// Pokud narazi konec stringu, tak vyhodi vyjimku.
// Pokud je neco spatne escapovane, tak vyhodi vyjimku.
//
// Na konci je input posunut na pozici za delimiter
std::string unescape_special_chars_and_advance(WeakStringInput &input, char delimiter);
std::string unescape_special_chars_and_advance(IOStreamInput &input, char delimiter);
std::string unescape_special_chars_and_advance(WeakString &input, char delimiter);

// Jako unescape_special_chars, jenom zahazujeme informaci, kde jsme skoncili
std::string unescape_special_chars(WeakString input, char delimiter);

// Preskoci mezery
// Prvni znak musi byt uvozovka
// Pak nasladuje string ukonceny uvozovkou, ktery odescapuje a vrati.
// Na konci je input posunut na pozici za ukoncovaci uvozovku.
std::string interpret_first_string_and_advance(WeakString &input);

gchar *cm_string_escape_string(WeakString input);

#endif // CM_STRING_H
