#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([texture], [1.0], [vladimir@stupin.su])
AC_CONFIG_SRCDIR([main.cpp])
AM_INIT_AUTOMAKE([foreign -Wportability -Wall -Werror -Wsyntax])
AC_CONFIG_FILES([Makefile])
AC_CANONICAL_TARGET

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdio.h stdlib.h string.h sys/stat.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset mkdir strcasecmp strrchr])

AC_OUTPUT
