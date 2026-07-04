#ifndef VERSION_H
#define VERSION_H

#ifdef PRODUCT_NAME

#include "project_version.hh"

static void version(int *argc, char ***argv, std::string const &user_string = "")
{
    VER::version(*argc, *argv, PRODUCT_NAME, user_string);
}

#else

#include <unistd.h>

#ifndef __USE_BSD
#define __USE_BSD
#endif

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <stdbool.h>
#include <stdarg.h>

enum VERSION_TYPE
{
	ELDIS_VERSION,
	FULL_VERSION,
	FILE_VERSION,
	GIT_VERSION
};

typedef struct
{
	int * argc;
	char *** argv;
	char * runtime_version;
	bool runtime_alocated;
	const char * progname;
} version_args;

#ifdef __cplusplus
extern "C" {
#endif

void __version (version_args info);
void __version_no_exit (version_args args);
void __version_exit_clean (void);
version_args __make_version_args (const size_t cnt, ...);
char * __get_versions (const char * s1, const char * s2);

#define _LIBVER_COUNT_ARGS(...) __LIBVER_COUNT_ARGS(0, ##__VA_ARGS__,9,8,7,6,5,4,3,2,1,0)
#define __LIBVER_COUNT_ARGS(_0_, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, cnt, ...) cnt

#define version(...) \
	__version(__make_version_args( \
		_LIBVER_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))
#define version_no_exit(...) \
	__version_no_exit(__make_version_args( \
		_LIBVER_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__))

#define version_with_progname(name, ...) { \
	version_args __version_args = __make_version_args( \
		_LIBVER_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__); \
	__version_args.progname = name; \
	__version(__version_args); \
}

#ifdef __cplusplus
}
#endif

#endif

#endif /* VERSION_H */
