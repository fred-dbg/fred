# AM_MAINTAINER_MDE([disable])

# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign

#HEADERS
#set the include path found by configure
INCLUDES= $(all_includes)

# targets:
noinst_LIBRARIES = libfredinternal.a
#bin_PROGRAMS = fred_read_log 
pkglib_PROGRAMS = fredhijack.so

# headers:
nobase_noinst_HEADERS = constants.h fred_wrappers.h util.h \
			synchronizationlogging.h log.h dmtcpmodule.h \
			trampolines.h fred_wrappers.h

libfredinternal_a_SOURCES = synchronizationlogging.cpp log.cpp fred.cpp \
			    fred_trampolines.cpp

fredhijack_so_SOURCES = fred_signalwrappers.cpp fred_epollwrappers.cpp \
			fred_mallocwrappers.cpp fred_filewrappers.cpp \
			fred_syscallsreal.c fred_socketwrappers.cpp \
			pthreadwrappers.cpp netwrappers.cpp

fredhijack_so_LDFLAGS   = -shared -module
fredhijack_so_LDADD     = libfredinternal.a -ldl -lpthread

#fred_read_log_SOURCES = fred_read_log.cpp nosyscallsreal.c 
#fred_read_log_LDADD   = libjalib.a libfredinternal.a fredhijack.so -lpthread


PICFLAGS=-fPIC
AM_CFLAGS = $(PICFLAGS)
AM_CXXFLAGS = $(PICFLAGS)
AM_LDFLAGS = $(PICFLAGS)

