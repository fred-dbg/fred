AC_INIT([FReD],[0.99],[],[FReD],[])
AC_PREREQ([2.60])
AM_INIT_AUTOMAKE([foreign])
# AM_MAINTAINER_MODE
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
                            [(DEPRECATED) provide dmtcp sources path])],
            [dmtcp_path=$withval],
            [dmtcp_path=no])

AC_ARG_WITH([dmtcp_root],
            [AS_HELP_STRING([--with-dmtcp-root=dir],
                            [provide root dir for DMTCP sources])],
            [dmtcp_root=$withval],
            [dmtcp_root=no])

# REMOVE THIS WHEN dmtcp_path goes away.
if test $dmtcp_root = no; then
  dmtcp_root=$dmtcp_path
fi

if test -d "$dmtcp_root/dmtcp/jalib"; then
  jalib_path="$dmtcp_root/dmtcp/jalib"
  dmtcp_src_path="$dmtcp_root/dmtcp/src"
  AC_SUBST([JALIB_PATH], [$jalib_path])
  AC_SUBST([DMTCP_SRC_PATH], [$dmtcp_src_path])
else
  AC_MSG_ERROR([Invalid dmtcp path, use --with-dmtcp-root=<dir>
		dir is root dir of DMTCP top-level source directory
		It is not necessary to first build DMTCP.])
fi

test -r "$dmtcp_root/config.log" && \
  grep -q '\$ .* --with-record-replay' $dmtcp_root/config.log || \
    AC_MSG_WARN([$dmtcp_root/config.log missing or not showing
                 --with-record-replay.  Make sure to use this module
                 with a DMTCP configured with:
                 [  ./configure --with-record-replay]])

AM_CONDITIONAL(HAS_I486,
  [test `uname -m` == i686 -o  `uname -m` == i586 -o `uname -m` == i486])

AC_DEFINE([DMTCP],[],[Always enable this.])

AC_OUTPUT
