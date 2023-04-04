#ifndef __STR__
#define __STR__

#include "bool.h"

#if defined(_WIN32) || defined(WIN32)
const int SYS_PATH_DELIM = '\\';
#else
const int SYS_PATH_DELIM = '/';
#endif

bool_t s_strcpy(char **pt, const char *s);
bool_t s_strncpy(char **pt, const char *s, size_t l);
bool_t s_strcat(char **pt, const char *s);
bool_t s_strncat(char **pt, const char *s, size_t l);
bool_t s_pathname(char **pathname, char *filename);
void s_free(char **pt);

#endif
