#ifndef _CMS_MISC_H
#define _CMS_MISC_H

#include "cms_header.h"

G_BEGIN_DECLS

// vypocte checksum pro sitovou komunikaci
unsigned short ipchecksum (const void * addr, int count);

// prevod casu utc casu na datum
gboolean time2ymdhms (time_t time, unsigned * year, unsigned * month, 
	unsigned * day, unsigned * hour, unsigned * min, unsigned * sec);
gboolean localtime2ymdhms (time_t time, unsigned * year, unsigned * month, 
	unsigned * day, unsigned * hour, unsigned * min, unsigned * sec);
gboolean ymdhms2time (unsigned year, unsigned month, unsigned day, 
	unsigned hour, unsigned min, unsigned sec, time_t * time);
gboolean ymdhms2localtime (unsigned year, unsigned month, unsigned day, 
	unsigned hour, unsigned min, unsigned sec, time_t * time);
gboolean time2text (time_t time, char * text);
gboolean time2texts (time_t time, char * text_date, char * text_time);
gboolean text2time (const char * text, time_t * time);
gboolean texts2time (const char * text_date, const char * text_time, time_t * time);

// otevre soubor, nerozlisuje velka a mala pismena ve filename
FILE * fopen_case (const char * filename, const char * mode);

// obdoba read a write, cteni/zapis probiha opakovane, 
// do dokud se nepodari zapsat/precist vse
ssize_t read_all (int fd, void * buffer, size_t count);
ssize_t write_all (int fd, const void * buffer, size_t count);

// zavola externi prikaz
int cms_run_command (const char * text, ...);

G_END_DECLS

#endif /* _CMS_MISC_H */
