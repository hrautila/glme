#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Gob Like Message Encoding], [0.2], [],
	[libglme], [https://github.com/hrautila/glme])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT


# Checks for programs.
AC_PROG_CC
##AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h stdint.h stddef.h ])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_UINT64_T

AC_CONFIG_FILES([Makefile
    src/Makefile
    test/Makefile
    perf/Makefile])

AC_OUTPUT
