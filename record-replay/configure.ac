AC_INIT([FReD],[0.99],[],[FReD],[])
AC_PREREQ([2.60])
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

AC_ARG_WITH([dmtcp_root],
            [AS_HELP_STRING([--with-dmtcp-root=dir],
                            [provide root dir for DMTCP sources])],
            [dmtcp_root=$withval],
            [dmtcp_root=no])

if test -d "$dmtcp_root"; then
  AC_SUBST([DMTCP_ROOT], [$dmtcp_root])
else
  AC_MSG_ERROR([Invalid dmtcp path; use --with-dmtcp-root=<DIR>
		DIR is the top-level directory of the DMTCP distribution.
		It is not necessary to first build DMTCP.])
fi

test -r "$dmtcp_root/config.log" && \
  grep -q '\$ .* --with-record-replay' $dmtcp_root/config.log || \
    AC_MSG_WARN([$dmtcp_root/config.log missing or not showing
                 --with-record-replay.  When you use this module, make sure
                 to use it with a DMTCP configured with:
                 [  ./configure --with-record-replay]])

AC_DEFINE([DMTCP],[],[Always enable this.])

AC_OUTPUT
