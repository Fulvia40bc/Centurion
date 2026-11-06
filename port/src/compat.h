#ifndef COMPAT_H
#define COMPAT_H
/* MSVC lacks strcasecmp and dirent.h; MinGW/clang have both. */
#ifdef _MSC_VER
#include <string.h>
#define strcasecmp _stricmp
#endif
#endif
