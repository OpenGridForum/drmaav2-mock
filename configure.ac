#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.68])
AC_INIT([DRMAAv2 Reference Implementation], [0.1], [drmaa-wg@ogf.org])
#AC_CONFIG_SRCDIR([drmaa2-debug.h])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcunit':
AC_CHECK_LIB([cunit], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([strdup])

# Checks for SQLite
AC_ARG_WITH([sqlite-include-path],
  [AS_HELP_STRING([--with-sqlite-include-path],
                  [location of the SQLite headers, defaults to /usr/include])],
  [SQLITE_CFLAGS="-I$withval"],
  [SQLITE_CFLAGS='-I/usr/include'])

AC_SUBST([SQLITE_CFLAGS])

AC_ARG_WITH([sqlite-lib-path],
  [AS_HELP_STRING([--with-sqlite-lib-path], [location of the SQLite libraries])],
  [SQLITE_LIBS="-L$withval -lsqlite3"],
  [SQLITE_LIBS='-lsqlite3'])
		
AC_SUBST([SQLITE_LIBS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

