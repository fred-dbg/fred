AC_INIT([FReD],[0.99],[],[FReD],[])
AC_PREREQ([2.60])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_CONFIG_FILES([Makefile])

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


AC_ARG_WITH([dmtcp_path],
            [AS_HELP_STRING([--with-dmtcp-path=path],
                            [provide dmtcp sources path])],
            [dmtcp_path=$withval],
            [dmtcp_path=no])

if test -d "$dmtcp_path/dmtcp/jalib"; then
  jalib_path="$dmtcp_path/dmtcp/jalib"
  AC_SUBST([JALIB_PATH], [$jalib_path])
else
  echo alskdfjalkdsfj "$dmtcp_path"
  AC_MSG_ERROR([Invalid dmtcp path, use --with-dmtcp-path=<path>])
fi

AC_DEFINE([DMTCP],[],[Always enable this.])

AC_OUTPUT
