/****************************************************************************
 * Copyright (C) 2009, 2010, 2011, 2012 by Kapil Arya, Gene Cooperman,      *
 *                                     Tyler Denniston, and Ana-Maria Visan *
 * {kapil,gene,tyler,amvisan}@ccs.neu.edu                                   *
 *                                                                          *
 * This file is part of FReD.                                               *
 *                                                                          *
 * FReD is free software: you can redistribute it and/or modify             *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * FReD is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with FReD.  If not, see <http://www.gnu.org/licenses/>.            *
 ****************************************************************************/

/* Ubuntu 11.10 and some other distros have a non-POSIX declaration of
 *   getnameinfo in /usr/include/netdb.h ("unsigned int flags").
 *   The netdb.h differs from 'man getnameinfo', which has the POSIX variant.
 * So, we declare the POSIX variant here, and play macro tricks to remove
 *   the potentially non-POSIX variant in netdb.h
 */
#include <unistd.h>
extern "C" int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                           char *host, size_t hostlen,
                           char *serv, size_t servlen, int flags);
#define getnameinfo(arg1,arg2,arg3,arg4,arg5,arg6,arg7) \
  dmtcp_getnameinfo_not_used(arg1,arg2,arg3,arg4,arg5,arg6,arg7)

#include <stdarg.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/version.h>
#include <limits.h>
#include "constants.h"
#include "fred_wrappers.h"
#include "util.h"
#include  "jassert.h"
#include  "jconvert.h"
#include  "jfilesystem.h"

#include "synchronizationlogging.h"
#include <sys/mman.h>
#include <sys/syscall.h>

/* We're done.  Now stop expanding getnameinfo. */
#undef getnameinfo


extern "C" int getsockname(int sockfd, struct sockaddr *addr,
                           socklen_t *addrlen)
{
  WRAPPER_EXECUTION_DISABLE_CKPT();
  WRAPPER_HEADER_CKPT_DISABLED(int, getsockname, _real_getsockname,
                               sockfd, addr, addrlen);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getsockname);
    if (retval != -1) {
      *addr = GET_FIELD(my_entry, getsockname, ret_addr);
      *addrlen = GET_FIELD(my_entry, getsockname, ret_addrlen);
    }
    WRAPPER_REPLAY_END(getsockname);
  } else if (SYNC_IS_RECORD) {
    retval = _real_getsockname(sockfd, addr, addrlen);
    if (retval != -1) {
      SET_FIELD2(my_entry, getsockname, ret_addr, *addr);
      SET_FIELD2(my_entry, getsockname, ret_addrlen, *addrlen);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  WRAPPER_EXECUTION_ENABLE_CKPT();
  return retval;
}

extern "C" int getpeername(int sockfd, struct sockaddr *addr,
                           socklen_t *addrlen)
{
  WRAPPER_EXECUTION_DISABLE_CKPT();
  WRAPPER_HEADER_CKPT_DISABLED(int, getpeername, _real_getpeername,
                               sockfd, addr, addrlen);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getpeername);
    if (retval != -1) {
      *addr = GET_FIELD(my_entry, getpeername, ret_addr);
      *addrlen = GET_FIELD(my_entry, getpeername, ret_addrlen);
    }
    WRAPPER_REPLAY_END(getpeername);
  } else if (SYNC_IS_RECORD) {
    retval = _real_getpeername(sockfd, addr, addrlen);
    if (retval != -1) {
      SET_FIELD2(my_entry, getpeername, ret_addr, *addr);
      SET_FIELD2(my_entry, getpeername, ret_addrlen, *addrlen);
    }
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  WRAPPER_EXECUTION_ENABLE_CKPT();
  return retval;
}

extern "C" struct passwd *getpwnam(const char *name)
{
  static __thread char buf[4096 * 4];
  static __thread struct passwd pwd;
  static __thread struct passwd *result;

  dmtcp::ThreadInfo::setOkToLogNextFnc();
  int res = getpwnam_r(name, &pwd, buf, sizeof(buf), &result);

  if (res == 0) {
    return result;
  }

  return NULL;
}

extern "C" struct passwd *getpwuid(uid_t uid)
{
  static __thread char buf[4096 * 4];
  static __thread struct passwd pwd;
  static __thread struct passwd *result;

  dmtcp::ThreadInfo::setOkToLogNextFnc();
  int res = getpwuid_r(uid, &pwd, buf, sizeof(buf), &result);

  if (res == 0) {
    return &pwd;
  }

  return NULL;
}

extern "C" struct group *getgrnam(const char *name)
{
  static __thread char buf[4096 * 4];
  static __thread struct group grp;
  static __thread struct group *result;

  dmtcp::ThreadInfo::setOkToLogNextFnc();
  int res = getgrnam_r(name, &grp, buf, sizeof(buf), &result);

  if (res == 0) {
    return &grp;
  }

  return NULL;
}

extern "C" struct group *getgrgid(gid_t gid)
{
  static __thread char buf[4096 * 4];
  static __thread struct group grp;
  static __thread struct group *result;

  dmtcp::ThreadInfo::setOkToLogNextFnc();
  int res = getgrgid_r(gid, &grp, buf, sizeof(buf), &result);

  if (res == 0) {
    return &grp;
  }

  return NULL;
}

extern "C" int getpwnam_r(const char *name, struct passwd *pwd,
                          char *buf, size_t buflen, struct passwd **result)
{
  WRAPPER_HEADER(int, getpwnam_r, _real_getpwnam_r, name, pwd, buf, buflen, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getpwnam_r);
    if (retval == 0 &&
        GET_FIELD(my_entry, getpwnam_r, ret_result) != NULL) {
      *pwd = GET_FIELD(my_entry, getpwnam_r, ret_pwd);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getpwnam_r, buf, buflen);
    }
    *result = GET_FIELD(my_entry, getpwnam_r, ret_result);
    WRAPPER_REPLAY_END(getpwnam_r);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getpwnam_r(name, pwd, buf, buflen, result);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval == 0 && result != NULL) {
      SET_FIELD2(my_entry, getpwnam_r, ret_pwd, *pwd);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getpwnam_r, buf, buflen);
    }
    SET_FIELD2(my_entry, getpwnam_r, ret_result, *result);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int getpwuid_r(uid_t uid, struct passwd *pwd,
                          char *buf, size_t buflen, struct passwd **result)
{
  WRAPPER_HEADER(int, getpwuid_r, _real_getpwuid_r, uid, pwd, buf, buflen, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getpwuid_r);
    if (retval == 0 &&
        GET_FIELD(my_entry, getpwuid_r, ret_result) != NULL) {
      *pwd = GET_FIELD(my_entry, getpwuid_r, ret_pwd);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getpwuid_r, buf, buflen);
    }
    *result = GET_FIELD(my_entry, getpwuid_r, ret_result);
    WRAPPER_REPLAY_END(getpwuid_r);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getpwuid_r(uid, pwd, buf, buflen, result);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval == 0 && result != NULL) {
      SET_FIELD2(my_entry, getpwuid_r, ret_pwd, *pwd);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getpwuid_r, buf, buflen);
    }
    SET_FIELD2(my_entry, getpwuid_r, ret_result, *result);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int getgrnam_r(const char *name, struct group *grp,
                          char *buf, size_t buflen, struct group **result)
{
  WRAPPER_HEADER(int, getgrnam_r, _real_getgrnam_r, name, grp, buf, buflen, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getgrnam_r);
    if (retval == 0 &&
        GET_FIELD(my_entry, getgrnam_r, ret_result) != NULL) {
      *grp = GET_FIELD(my_entry, getgrnam_r, ret_grp);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getgrnam_r, buf, buflen);
    }
    *result = GET_FIELD(my_entry, getgrnam_r, ret_result);
    WRAPPER_REPLAY_END(getgrnam_r);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getgrnam_r(name, grp, buf, buflen, result);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval == 0 && result != NULL) {
      SET_FIELD2(my_entry, getgrnam_r, ret_grp, *grp);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getgrnam_r, buf, buflen);
    }
    SET_FIELD2(my_entry, getgrnam_r, ret_result, *result);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" int getgrgid_r(gid_t gid, struct group *grp,
                          char *buf, size_t buflen, struct group **result)
{
  WRAPPER_HEADER(int, getgrgid_r, _real_getgrgid_r, gid, grp, buf, buflen, result);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getgrgid_r);
    if (retval == 0 &&
        GET_FIELD(my_entry, getgrgid_r, ret_result) != NULL) {
      *grp = GET_FIELD(my_entry, getgrgid_r, ret_grp);
      WRAPPER_REPLAY_READ_FROM_READ_LOG(getgrgid_r, buf, buflen);
    }
    *result = GET_FIELD(my_entry, getgrgid_r, ret_result);
    WRAPPER_REPLAY_END(getgrgid_r);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getgrgid_r(gid, grp, buf, buflen, result);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    if (retval == 0 && result != NULL) {
      SET_FIELD2(my_entry, getgrgid_r, ret_grp, *grp);
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getgrgid_r, buf, buflen);
    }
    SET_FIELD2(my_entry, getgrgid_r, ret_result, *result);
    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" char *getlogin(void)
{
  const size_t buflen = 1024;
  static __thread char buf[buflen];
  int res = getlogin_r(buf, buflen);
  if (res != 0) {
    return NULL;
  }
  return buf;
}

extern "C" int getlogin_r(char *name, size_t bufsize)
{
  uid_t uid = getuid();
  const size_t buflen = 1024;
  struct passwd pwd;
  struct passwd *tpwd;
  int result = 0;
  int res;
  char buf[buflen];

  dmtcp::ThreadInfo::setOkToLogNextFnc();
  res = getpwuid_r(uid, &pwd, buf, buflen, &tpwd);
  JASSERT(res == 0 || errno != ERANGE);

  if (strlen(pwd.pw_name) > bufsize) {
    errno = ERANGE;
    result = ERANGE;
  } else {
    strcpy(name, pwd.pw_name);
  }
  return result;
}


#define ADDRINFO_MAX_RES 32
struct addrinfo_extended {
  struct addrinfo *_addrinfo_p;
  struct addrinfo _addrinfo;
  struct sockaddr _sockaddr;
  char canonname[256];
};

extern "C" int getaddrinfo(const char *node, const char *service,
                           const struct addrinfo *hints, struct addrinfo **res)
{
  struct addrinfo_extended addrinfo_res[ADDRINFO_MAX_RES];
  struct addrinfo *rp;
  int numResults = 0;

  WRAPPER_HEADER(int, getaddrinfo, _real_getaddrinfo, node, service, hints,
                 res);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getaddrinfo);
    if (retval == 0) {
      *res = GET_FIELD(my_entry, getaddrinfo, ret_res);
      numResults = GET_FIELD(my_entry, getaddrinfo, num_results);

      WRAPPER_REPLAY_READ_FROM_READ_LOG(getaddrinfo, (void*) addrinfo_res,
                                        (numResults *
                                         sizeof (struct addrinfo_extended)));
      for (int i = 0; i < numResults; i++) {
        struct addrinfo_extended *ext_info = &addrinfo_res[i];
        struct addrinfo *_addrinfo = &(addrinfo_res[i]._addrinfo);
        struct sockaddr *_sockaddr = &(addrinfo_res[i]._sockaddr);
        memcpy(ext_info->_addrinfo_p, _addrinfo, sizeof(struct addrinfo));
        memcpy(_addrinfo->ai_addr, _sockaddr, _addrinfo->ai_addrlen);
        if (_addrinfo->ai_canonname != NULL) {
          strncpy(_addrinfo->ai_canonname, ext_info->canonname,
                  sizeof(ext_info->canonname));
        }
      }
    }
    WRAPPER_REPLAY_END(getaddrinfo);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getaddrinfo(node, service, hints, res);
    dmtcp::ThreadInfo::unsetOptionalEvent();

    if (retval == 0) {
      SET_FIELD2(my_entry, getaddrinfo, ret_res, *res);
      for (rp = *res; rp != NULL; rp = rp->ai_next) {
        JASSERT(numResults < ADDRINFO_MAX_RES);
        struct addrinfo_extended *ext_info = &addrinfo_res[numResults];
        struct addrinfo *_addrinfo = &(addrinfo_res[numResults]._addrinfo);
        struct sockaddr *_sockaddr = &(addrinfo_res[numResults]._sockaddr);
        ext_info->_addrinfo_p = rp;
        memcpy(_addrinfo, rp, sizeof (struct addrinfo));
        memcpy(_sockaddr, rp->ai_addr, rp->ai_addrlen);
        if (rp->ai_canonname != NULL) {
          strncpy(ext_info->canonname, rp->ai_canonname,
                  sizeof(ext_info->canonname));
        }
        numResults++;
      }
      WRAPPER_LOG_WRITE_INTO_READ_LOG(getaddrinfo, (void*) addrinfo_res,
                                      (numResults *
                                       sizeof (struct addrinfo_extended)));
    }
    SET_FIELD2(my_entry, getaddrinfo, num_results, numResults);

    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;
}

extern "C" void freeaddrinfo(struct addrinfo *res)
{
  WRAPPER_HEADER_VOID(freeaddrinfo, _real_freeaddrinfo, res);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_VOID(freeaddrinfo);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    _real_freeaddrinfo(res);
    dmtcp::ThreadInfo::unsetOptionalEvent();
    WRAPPER_LOG_WRITE_ENTRY_VOID(my_entry);
  }
}

extern "C" int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                           char *host, size_t hostlen,
                           char *serv, size_t servlen, int flags)
{
  WRAPPER_HEADER(int, getnameinfo, _real_getnameinfo, sa, salen, host, hostlen,
                 serv, servlen, flags);
  if (SYNC_IS_REPLAY) {
    WRAPPER_REPLAY_START(getnameinfo);
    if (retval == 0 && host != NULL) {
      strncpy(host, GET_FIELD(my_entry, getnameinfo, ret_host), hostlen);
    }
    if (retval == 0 && host != NULL) {
      strncpy(serv, GET_FIELD(my_entry, getnameinfo, ret_serv), servlen);
    }
    WRAPPER_REPLAY_END(getnameinfo);
  } else if (SYNC_IS_RECORD) {
    dmtcp::ThreadInfo::setOptionalEvent();
    retval = _real_getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
    dmtcp::ThreadInfo::unsetOptionalEvent();

    if (retval == 0 && host != NULL) {
      strncpy(GET_FIELD(my_entry, getnameinfo, ret_host), host, hostlen);
    }
    if (retval == 0 && host != NULL) {
      strncpy(GET_FIELD(my_entry, getnameinfo, ret_serv), serv, servlen);
    }

    WRAPPER_LOG_WRITE_ENTRY(my_entry);
  }
  return retval;

}
