#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

void s_free(char **t)
{
    if (*t != NULL)
    {
        free(*t);
        *t = NULL;
    }
}

bool_t s_let(char **t, char *s)
{
    if (s == NULL)
    {
        fprintf(stderr, "s_let: s == NULL.\n");
        return FALSE;
    }
    if (t == NULL)
    {
        free(s);
        fprintf(stderr, "s_let: t == NULL.\n");
        return FALSE;
    }
    s_free(t);
    *t = s;
    return TRUE;
}

char *sn_strcpy(const char *s)
{
    size_t l;
    char *t;

    if (s == NULL)
    {
        fprintf(stderr, "sn_strcpy: s == NULL.\n");
        return NULL;
    }

    l = strlen(s);

    t = (char *)malloc(l + 1);
    if (t == NULL)
    {
        fprintf(stderr, "s_strcpy: malloc failed.\n");
        return NULL;
    }
    strcpy(t, s);
    return t;
}

char *sn_strncpy(const char *s, size_t l)
{
    char *t;

    if (s == NULL)
    {
        fprintf(stderr, "sn_strncpy: s == NULL.\n");
        return NULL;
    }

    t = (char *)malloc(l + 1);
    if (t == NULL)
    {
        fprintf(stderr, "sn_strncpy: malloc failed.\n");
        return NULL;
    }

    memcpy(t, s, l);
    t[l] = '\0';
    return t;
}

char *sn_strcat(const char *s0, const char *s1)
{
    char *t;
    size_t l0;
    size_t l1;

    if (s0 == NULL)
    {
        fprintf(stderr, "sn_strcat: s0 == NULL.\n");
        return NULL;
    }
    if (s1 == NULL)
    {
        fprintf(stderr, "sn_strcat: s1 == NULL.\n");
        return NULL;
    }

    l0 = strlen(s0);
    l1 = strlen(s1);
    t = (char *)malloc(l0 + l1 + 1);
    if (t == NULL)
    {
        fprintf(stderr, "sn_strcat: malloc failed.\n");
        return NULL;
    }

    memcpy(t, s0, l0);
    memcpy(&(t[l0]), s1, l1);
    t[l0 + l1] = '\0';
    return t;
}

char *sn_strncat(const char *s0, const char *s1, size_t l)
{
    char *t;
    size_t l0;

    if (s0 == NULL)
    {
        fprintf(stderr, "sn_strncat: s0 == NULL.\n");
        return NULL;
    }
    if (s1 == NULL)
    {
        fprintf(stderr, "sn_strncat: s1 == NULL.\n");
        return NULL;
    }

    l0 = strlen(s0);
    t = (char *)malloc(l0 + l + 1);
    if (t == NULL)
    {
        fprintf(stderr, "sn_strncat: malloc failed.\n");
        return NULL;
    }

    memcpy(t, s0, l0);
    memcpy(&(t[l0]), s1, l);
    t[l0 + l] = '\0';
    return t;
}

bool_t s_strcpy(char **t, const char *s)
{
    return s_let(t, sn_strcpy(s));
}

bool_t s_strncpy(char **t, const char *s, size_t l)
{
    return s_let(t, sn_strncpy(s, l));
}

bool_t s_strcat(char **t, const char *s)
{
    if (t == NULL)
    {
        fprintf(stderr, "s_strcat: t == NULL.\n");
        return FALSE;
    }
    return s_let(t, sn_strcat(*t, s));
}

bool_t s_strncat(char **t, const char *s, size_t l)
{
    if (t == NULL)
    {
        fprintf(stderr, "s_strncat: t == NULL.\n");
        return FALSE;
    }
    return s_let(t, sn_strncat(*t, s, l));
}

char *sn_pathname(char *filename)
{
    char *b;
    char *e;

    if (filename == NULL)
    {
        fprintf(stderr, "sn_pathname: filename == NULL.\n");
        return NULL;
    }

    b = strrchr(filename, SYS_PATH_DELIM);
    if (b == NULL)
        b = filename;
    else
        b = &(b[1]);
    e = strrchr(b, '.');
    if (e == NULL)
        return sn_strcpy(filename);
    return sn_strncpy(filename, e - filename);
}

bool_t s_pathname(char **pathname, char *filename)
{
    return s_let(pathname, sn_pathname(filename));
}
