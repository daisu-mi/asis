#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(asisd, 2.0.0, daisu-mi@nc.u-tokyo.ac.jp)
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)
AC_CONFIG_SRCDIR([src/asisd.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_INSTALL

AM_PROG_CC_C_O

LIBS="$LIBS -L/usr/local/lib"
CFLAGS="$CFLAGS -I/usr/local/include"

AC_HAVE_LIBRARY(event,
  ,
  [AC_MSG_ERROR([Cannot find libevent: do you have the libevent development packages installed?])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([memset socket strchr strncasecmp])

# Checks for my enable functions.
AC_ARG_ENABLE(debug,
[  --enable-debug          enable debugging [default no]])
if test "$enable_debug" = "yes"; then
  CFLAGS="-DDEBUG $CFLAGS"
fi

AC_ARG_ENABLE(setuid,
[  --enable-setuid         enable setuid [default no]])
if test "$enable_setuid" = "yes"; then
  CFLAGS="-DUSE_SETUID $CFLAGS"
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
