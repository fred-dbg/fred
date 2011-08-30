AC_INIT([FReD],[0.99],[],[FReD],[])
AC_PREREQ([2.60])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_LANG_CPLUSPLUS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_C_O
AC_PROG_RANLIB

AC_ARG_ENABLE([debug],
            [AS_HELP_STRING([--enable-debug],
                            [enable (very) verbose debug output
                             and write log files to \$DMTCP_TMPDIR (default is
                             disabled)])],
            [use_jassert=$enableval],
            [use_jassert=no])

if test "$use_jassert" = "yes"; then
  AC_DEFINE([DEBUG],[],[Verbose debug output and log files in $DMTCP_TMPDIR])
  CFLAGS+=" -g -O0"
  CXXFLAGS+=" -g -O0"
fi


AC_OUTPUT(Makefile)
