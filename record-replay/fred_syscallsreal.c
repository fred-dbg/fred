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


#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
// These next two are defined in features.h based on the user macros above.
// #define GNU_SRC
// #define __USE_UNIX98

#include <malloc.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "constants.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include "fred_wrappers.h"
#include "trampolines.h"
#include <sys/mman.h>
#include <dirent.h>
#include <time.h>
#define open _libc_open
#include <fcntl.h>
#undef open

#define NOT_IMPLEMENTED()                                                   \
  do {                                                                      \
    fprintf(stderr, "NOT REACHED***********\n\n\n");                        \
    _exit(0);                                                               \
  } while(0)

LIB_PRIVATE void *get_real_func_addr(event_code_t e, const char *name);
LIB_PRIVATE void fred_get_libc_func_addr();

static int _wrappers_initialized = 0;
LIB_PRIVATE void *_real_func_addr[numTotalWrappers];

static char wrapper_init_buf[1024];
static trampoline_info_t pthread_getspecific_trampoline_info;
void *_fred_pthread_getspecific(pthread_key_t key)
{
  if (_wrappers_initialized) {
    fprintf(stderr, "DMTCP INTERNAL ERROR\n\n");
    abort();
  }
  pthread_setspecific(key, wrapper_init_buf);
  UNINSTALL_TRAMPOLINE(pthread_getspecific_trampoline_info);
  return pthread_getspecific(key);
}

static void _fred_PreparePthreadGetSpecific()
{
  dmtcp_setup_trampoline_by_addr(&pthread_getspecific,
                                 (void*) &_fred_pthread_getspecific,
                                 &pthread_getspecific_trampoline_info);
}

LIB_PRIVATE
void initialize_wrappers()
{
  if (!_wrappers_initialized) {
    _fred_PreparePthreadGetSpecific();
    fred_get_libc_func_addr();
    _wrappers_initialized = 1;
  }
}

LIB_PRIVATE
void *get_real_func_addr(event_code_t e, const char *name) {
  if (_real_func_addr[e] == NULL) {
    prepareFredWrappers();
  }
  if (_real_func_addr[e] == NULL) {
    fprintf(stderr, "*** DMTCP: Error: lookup failed for %s.\n"
                    "           The symbol wasn't found in current library"
                    " loading sequence.\n"
                    "    Aborting.\n", name);
    abort();
  }
  return _real_func_addr[e];
}

LIB_PRIVATE
void *_real_dlsym(void *handle, const char *symbol) {
  typedef void* ( *fncptr ) (void *handle, const char *symbol);
  fncptr dlsym_fptr = NULL;

  if (dlsym_fptr == 0) {
    dlsym_fptr = dmtcp_get_libc_dlsym_addr();
    if (dlsym_fptr == NULL) {
      fprintf(stderr, "DMTCP: Internal Error: Not Reached\n");
      abort();
    }
  }

  return (*dlsym_fptr) ( handle, symbol );
}

//////////////////////////
//// FIRST DEFINE REAL VERSIONS OF NEEDED FUNCTIONS

#define REAL_FUNC_PASSTHROUGH(name)  REAL_FUNC_PASSTHROUGH_TYPED(int, name)

#define REAL_FUNC_PASSTHROUGH_TYPED(type,name) \
  static type (*fn)() = NULL; \
  if (fn == NULL) { \
    fn = get_real_func_addr(name##_event, #name); \
  } \
  return (*fn)

#define REAL_FUNC_PASSTHROUGH_VOID(name) \
  static void (*fn)() = NULL; \
  if (fn == NULL) { \
    fn = get_real_func_addr(name##_event, #name); \
  } \
  (*fn)


LIB_PRIVATE
int _real_pthread_mutex_lock(pthread_mutex_t *mutex) {
  int i = 0;
  i++;
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_mutex_lock ) ( mutex );
}

LIB_PRIVATE
int _real_pthread_mutex_trylock(pthread_mutex_t *mutex) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_mutex_trylock ) ( mutex );
}

LIB_PRIVATE
int _real_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_mutex_unlock ) ( mutex );
}

LIB_PRIVATE
int _real_pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_rwlock_unlock ) ( rwlock );
}

LIB_PRIVATE
int _real_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_rwlock_rdlock ) ( rwlock );
}

LIB_PRIVATE
int _real_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_rwlock_wrlock ) ( rwlock );
}

LIB_PRIVATE
ssize_t _real_read(int fd, void *buf, size_t count) {
  REAL_FUNC_PASSTHROUGH ( read ) ( fd,buf,count );
}

LIB_PRIVATE
ssize_t _real_write(int fd, const void *buf, size_t count) {
  REAL_FUNC_PASSTHROUGH_TYPED ( ssize_t,write ) ( fd,buf,count );
}

LIB_PRIVATE
int _real_select(int nfds, fd_set *readfds, fd_set *writefds,
                 fd_set *exceptfds, struct timeval *timeout) {
  REAL_FUNC_PASSTHROUGH ( select ) ( nfds,readfds,writefds,exceptfds,timeout );
}

LIB_PRIVATE
int _real_ppoll(struct pollfd *fds, nfds_t nfds,
               const struct timespec *timeout_ts, const sigset_t *sigmask) {
  REAL_FUNC_PASSTHROUGH(ppoll) (fds, nfds, timeout_ts, sigmask);
}

LIB_PRIVATE
int _real_socket ( int domain, int type, int protocol )
{
  REAL_FUNC_PASSTHROUGH ( socket ) ( domain,type,protocol );
}

LIB_PRIVATE
int _real_connect ( int sockfd, const struct sockaddr *serv_addr,
                    socklen_t addrlen )
{
  REAL_FUNC_PASSTHROUGH ( connect ) ( sockfd,serv_addr,addrlen );
}

LIB_PRIVATE
int _real_bind ( int sockfd, const struct sockaddr *my_addr,
                 socklen_t addrlen )
{
  REAL_FUNC_PASSTHROUGH ( bind ) ( sockfd,my_addr,addrlen );
}

LIB_PRIVATE
int _real_listen ( int sockfd, int backlog )
{
  REAL_FUNC_PASSTHROUGH ( listen ) ( sockfd,backlog );
}

LIB_PRIVATE
struct tm * _real_localtime_r ( const time_t *timep, struct tm *result )
{
  REAL_FUNC_PASSTHROUGH_TYPED ( struct tm *, localtime_r ) ( timep, result );
}

int _real_utime ( const char *filename, const struct utimbuf *times )
{
  REAL_FUNC_PASSTHROUGH_TYPED ( int, utime ) ( filename, times );
}

LIB_PRIVATE
int _real_utimes ( const char *filename, const struct timeval *times )
{
  REAL_FUNC_PASSTHROUGH_TYPED ( int, utimes ) ( filename, times );
}

LIB_PRIVATE
int _real_clock_getres(clockid_t clk_id, struct timespec *res)
{
  REAL_FUNC_PASSTHROUGH(clock_getres) (clk_id, res);
}

LIB_PRIVATE
int _real_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
  REAL_FUNC_PASSTHROUGH(clock_gettime) (clk_id, tp);
}

LIB_PRIVATE
int _real_clock_settime(clockid_t clk_id, const struct timespec *tp)
{
  REAL_FUNC_PASSTHROUGH(clock_settime) (clk_id, tp);
}

LIB_PRIVATE
int _real_accept ( int sockfd, struct sockaddr *addr, socklen_t *addrlen )
{
  REAL_FUNC_PASSTHROUGH ( accept ) ( sockfd,addr,addrlen );
}

LIB_PRIVATE
int _real_accept4 ( int sockfd, struct sockaddr *addr, socklen_t *addrlen,
                    int flags )
{
  REAL_FUNC_PASSTHROUGH ( accept4 ) ( sockfd,addr,addrlen,flags );
}

LIB_PRIVATE
int _real_setsockopt ( int s, int level, int optname, const void *optval,
                       socklen_t optlen )
{
  REAL_FUNC_PASSTHROUGH ( setsockopt ) ( s,level,optname,optval,optlen );
}

LIB_PRIVATE
int _real_getsockopt ( int s, int level, int optname, void *optval,
                       socklen_t *optlen )
{
  REAL_FUNC_PASSTHROUGH ( getsockopt ) ( s,level,optname,optval,optlen );
}

LIB_PRIVATE
int _real_close ( int fd )
{
  REAL_FUNC_PASSTHROUGH ( close ) ( fd );
}

LIB_PRIVATE
int _real_chmod ( const char *path, mode_t mode )
{
  REAL_FUNC_PASSTHROUGH ( chmod ) ( path, mode );
}

LIB_PRIVATE
int _real_chown ( const char *path, uid_t owner, gid_t group )
{
  REAL_FUNC_PASSTHROUGH ( chown ) ( path, owner, group );
}

LIB_PRIVATE
int _real_fclose ( FILE *fp )
{
  REAL_FUNC_PASSTHROUGH ( fclose ) ( fp );
}

LIB_PRIVATE
int _real_fchdir ( int fd )
{
  REAL_FUNC_PASSTHROUGH ( fchdir ) ( fd );
}

LIB_PRIVATE
int _real_getpt ( void )
{
  NOT_IMPLEMENTED();
}

LIB_PRIVATE
int _real_ptsname_r ( int fd, char * buf, size_t buflen )
{
  NOT_IMPLEMENTED();
}

LIB_PRIVATE
int _real_socketpair ( int d, int type, int protocol, int sv[2] )
{
  REAL_FUNC_PASSTHROUGH ( socketpair ) ( d,type,protocol,sv );
}

LIB_PRIVATE
void _real_openlog ( const char *ident, int option, int facility )
{
  NOT_IMPLEMENTED();
}

LIB_PRIVATE
void _real_closelog ( void )
{
  NOT_IMPLEMENTED();
}

//set the handler
LIB_PRIVATE
sighandler_t _real_signal(int signum, sighandler_t handler){
  REAL_FUNC_PASSTHROUGH_TYPED ( sighandler_t, signal ) (signum, handler);
}
LIB_PRIVATE
int _real_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact){
  REAL_FUNC_PASSTHROUGH ( sigaction ) ( signum, act, oldact );
}
sighandler_t _real_sigset(int sig, sighandler_t disp)
{
  REAL_FUNC_PASSTHROUGH_TYPED ( sighandler_t, sigset ) ( sig, disp );
}

LIB_PRIVATE
int _real_sigwait(const sigset_t *set, int *sig) {
  REAL_FUNC_PASSTHROUGH ( sigwait ) ( set, sig);
}

LIB_PRIVATE
int _real_ioctl(int d, int request, void *arg) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, ioctl ) ( d, request, arg );
}

LIB_PRIVATE
int   _real_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
  REAL_FUNC_PASSTHROUGH ( waitid ) ( idtype, id, infop, options );
}

LIB_PRIVATE
pid_t _real_wait4(pid_t pid, __WAIT_STATUS status, int options, struct rusage *rusage) {
  REAL_FUNC_PASSTHROUGH_TYPED ( pid_t, wait4 ) ( pid, status, options, rusage );
}

LIB_PRIVATE
int _real_open( const char *pathname, int flags, mode_t mode) {
  REAL_FUNC_PASSTHROUGH ( open ) ( pathname, flags, mode );
}

LIB_PRIVATE
int _real_open64( const char *pathname, int flags, mode_t mode) {
  REAL_FUNC_PASSTHROUGH ( open ) ( pathname, flags, mode );
}

LIB_PRIVATE
FILE * _real_fopen( const char *path, const char *mode ) {
  REAL_FUNC_PASSTHROUGH_TYPED ( FILE *, fopen ) ( path, mode );
}

LIB_PRIVATE
FILE * _real_fopen64( const char *path, const char *mode ) {
  REAL_FUNC_PASSTHROUGH_TYPED ( FILE *, fopen64 ) ( path, mode );
}

LIB_PRIVATE
FILE * _real_freopen( const char *path, const char *mode, FILE *stream ) {
  REAL_FUNC_PASSTHROUGH_TYPED ( FILE *, freopen ) ( path, mode, stream );
}

LIB_PRIVATE
int _real_xstat(int vers, const char *path, struct stat *buf) {
  REAL_FUNC_PASSTHROUGH ( xstat ) ( vers, path, buf );
}

LIB_PRIVATE
int _real_xstat64(int vers, const char *path, struct stat64 *buf) {
  REAL_FUNC_PASSTHROUGH ( xstat64 ) ( vers, path, buf );
}

LIB_PRIVATE
int _real_lxstat(int vers, const char *path, struct stat *buf) {
  REAL_FUNC_PASSTHROUGH ( lxstat ) ( vers, path, buf );
}

LIB_PRIVATE
int _real_lxstat64(int vers, const char *path, struct stat64 *buf) {
  REAL_FUNC_PASSTHROUGH ( lxstat64 ) ( vers, path, buf );
}

LIB_PRIVATE
int _real_fxstat(int vers, int fd, struct stat *buf) {
  REAL_FUNC_PASSTHROUGH ( fxstat ) ( vers, fd, buf );
}

LIB_PRIVATE
int _real_fxstat64(int vers, int fd, struct stat64 *buf) {
  REAL_FUNC_PASSTHROUGH ( fxstat64 ) ( vers, fd, buf );
}

LIB_PRIVATE
ssize_t _real_readlink(const char *path, char *buf, size_t bufsiz) {
  REAL_FUNC_PASSTHROUGH_TYPED ( ssize_t, readlink ) ( path, buf, bufsiz );
}

LIB_PRIVATE
char *_real_realpath(const char *path, char *resolved_path) {
  REAL_FUNC_PASSTHROUGH_TYPED (char*, realpath) (path, resolved_path);
}

LIB_PRIVATE
int _real_pthread_join(pthread_t thread, void **value_ptr) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, pthread_join ) ( thread, value_ptr );
}

LIB_PRIVATE
int _real_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine)(void*), void *arg) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_create )
    (thread,attr,start_routine,arg);
}

LIB_PRIVATE
void * _real_calloc(size_t nmemb, size_t size) {
  REAL_FUNC_PASSTHROUGH_TYPED(void*, calloc) (nmemb, size);
}

LIB_PRIVATE
void * _real_malloc(size_t size) {
  REAL_FUNC_PASSTHROUGH_TYPED (void*, malloc) (size);
}

LIB_PRIVATE
void * _real_realloc(void *ptr, size_t size) {
  REAL_FUNC_PASSTHROUGH_TYPED (void*, realloc) (ptr, size);
}

LIB_PRIVATE
void * _real_libc_memalign(size_t boundary, size_t size) {
  REAL_FUNC_PASSTHROUGH_TYPED (void*, libc_memalign) (boundary, size);
}

LIB_PRIVATE
void _real_free(void *ptr) {
  REAL_FUNC_PASSTHROUGH_VOID (free) (ptr);
}

LIB_PRIVATE
void *_real_mmap(void *addr, size_t length, int prot, int flags,
    int fd, off_t offset) {
  return (void*) _real_syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
  //REAL_FUNC_PASSTHROUGH_TYPED (void*, mmap) (addr,length,prot,flags,fd,offset);
}

LIB_PRIVATE
void *_real_mmap64(void *addr, size_t length, int prot, int flags,
    int fd, off64_t offset) {
  return (void*) _real_syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
  //REAL_FUNC_PASSTHROUGH_TYPED (void*,mmap64) (addr,length,prot,flags,fd,offset);
}

LIB_PRIVATE
void *_real_mremap(void *old_address, size_t old_size, size_t new_size,
    int flags, void *new_address) {
  return (void*) _real_syscall(SYS_mremap, old_address, old_size, new_size,
                               flags, new_address);
  //REAL_FUNC_PASSTHROUGH_TYPED (void*, mremap)
    //(old_address, old_size, new_size, flags, new_address);
}

LIB_PRIVATE
int _real_munmap(void *addr, size_t length) {
  return (int) _real_syscall(SYS_munmap, addr, length);
  //REAL_FUNC_PASSTHROUGH_TYPED (int, munmap) (addr, length);
}

LIB_PRIVATE
int _real_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, getsockname ) ( sockfd, addr, addrlen );
}

LIB_PRIVATE
int _real_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, getpeername ) ( sockfd, addr, addrlen );
}

LIB_PRIVATE
int _real_closedir(DIR *dirp) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, closedir ) ( dirp );
}

LIB_PRIVATE
int _real_openat(int dirfd, const char *pathname, int flags) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, openat ) ( dirfd, pathname, flags );
}

LIB_PRIVATE
DIR * _real_fdopendir(int fd) {
  REAL_FUNC_PASSTHROUGH_TYPED ( DIR *, fdopendir ) ( fd );
}

LIB_PRIVATE
DIR * _real_opendir(const char *name) {
  REAL_FUNC_PASSTHROUGH_TYPED ( DIR *, opendir ) ( name );
}

LIB_PRIVATE
int _real_mkdir(const char *pathname, mode_t mode) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, mkdir ) ( pathname, mode );
}

LIB_PRIVATE
int _real_mkstemp(char *temp) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, mkstemp ) ( temp );
}

LIB_PRIVATE
FILE * _real_fdopen(int fd, const char *mode) {
  REAL_FUNC_PASSTHROUGH_TYPED ( FILE *, fdopen ) ( fd, mode );
}

LIB_PRIVATE
char * _real_fgets(char *s, int size, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( char *, fgets ) ( s, size, stream );
}

LIB_PRIVATE
int _real_ferror(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, ferror ) ( stream );
}

LIB_PRIVATE
int _real_feof(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, feof ) ( stream );
}

LIB_PRIVATE
int _real_fileno(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fileno ) ( stream );
}

LIB_PRIVATE
int _real_fflush(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fflush ) ( stream );
}

LIB_PRIVATE
int _real_setvbuf(FILE *stream, char *buf, int mode, size_t size) {
  REAL_FUNC_PASSTHROUGH_TYPED(int, setvbuf) (stream, buf, mode, size);
}

LIB_PRIVATE
int _real_fdatasync(int fd) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fdatasync ) ( fd );
}

LIB_PRIVATE
int _real_fsync(int fd) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fsync ) ( fd );
}

LIB_PRIVATE
int _real_fseek(FILE *stream, long offset, int whence) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fseek ) ( stream, offset, whence );
}

LIB_PRIVATE
int _real_getc(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, getc ) ( stream );
}

LIB_PRIVATE
char * _real_getcwd(char *buf, size_t size) {
  REAL_FUNC_PASSTHROUGH_TYPED ( char *, getcwd ) ( buf, size );
}

LIB_PRIVATE
int _real_gettimeofday(struct timeval *tv, struct timezone *tz) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, gettimeofday ) ( tv, tz );
}

LIB_PRIVATE
int _real_fgetc(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fgetc ) ( stream );
}

LIB_PRIVATE
int _real_ungetc(int c, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, ungetc ) ( c, stream );
}

LIB_PRIVATE
ssize_t _real_getline(char **lineptr, size_t *n, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( ssize_t, getline ) ( lineptr, n, stream );
}

LIB_PRIVATE
ssize_t _real_getdelim(char **lineptr, size_t *n, int delim, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( ssize_t, getdelim ) ( lineptr, n, delim, stream );
}

LIB_PRIVATE
int _real_link(const char *oldpath, const char *newpath) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, link ) ( oldpath, newpath );
}

LIB_PRIVATE
int _real_symlink(const char *oldpath, const char *newpath) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, symlink ) ( oldpath, newpath );
}

LIB_PRIVATE
int _real_rename(const char *oldpath, const char *newpath) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, rename ) ( oldpath, newpath );
}

LIB_PRIVATE
void _real_rewind(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_VOID ( rewind ) ( stream );
}

LIB_PRIVATE
int _real_rmdir(const char *pathname) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, rmdir ) ( pathname );
}

LIB_PRIVATE
long _real_ftell(FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( long, ftell ) ( stream );
}

LIB_PRIVATE
int _real_fgetpos(FILE *stream, fpos_t *pos) {
  REAL_FUNC_PASSTHROUGH ( fgetpos ) ( stream, pos );
 }

LIB_PRIVATE
int _real_fgetpos64(FILE *stream, fpos64_t *pos) {
  REAL_FUNC_PASSTHROUGH ( fgetpos64 ) ( stream, pos );
}

LIB_PRIVATE
int _real_fsetpos(FILE *stream, const fpos_t *pos) {
  REAL_FUNC_PASSTHROUGH ( fsetpos ) ( stream, pos );
}

LIB_PRIVATE
int _real_fsetpos64(FILE *stream, const fpos64_t *pos) {
  REAL_FUNC_PASSTHROUGH ( fsetpos64 ) ( stream, pos );
}


LIB_PRIVATE
int _real_fputs(const char *s, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fputs ) ( s, stream );
}

LIB_PRIVATE
int _real_fputc(int c, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fputc ) ( c, stream );
}

LIB_PRIVATE
int _real_putc(int c, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, putc ) ( c, stream );
}

LIB_PRIVATE
size_t _real_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( size_t, fwrite) ( ptr, size, nmemb, stream );
}

LIB_PRIVATE
size_t _real_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  REAL_FUNC_PASSTHROUGH_TYPED ( size_t, fread) ( ptr, size, nmemb, stream );
}

LIB_PRIVATE
int _real_fcntl(int fd, int cmd, void *arg) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, fcntl ) ( fd, cmd, arg);
}

LIB_PRIVATE
int _real_pthread_cond_signal(pthread_cond_t *cond) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_cond_signal ) ( cond );
}

LIB_PRIVATE
int _real_pthread_cond_broadcast(pthread_cond_t *cond) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_cond_broadcast ) ( cond );
}

LIB_PRIVATE
int _real_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_cond_wait ) ( cond,mutex );
}

LIB_PRIVATE
int _real_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
    const struct timespec *abstime) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_cond_timedwait ) ( cond,mutex,abstime );
}

LIB_PRIVATE
int _real_pthread_cond_destroy(pthread_cond_t *cond) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_cond_destroy ) ( cond );
}

LIB_PRIVATE
void _real_pthread_exit(void *value_ptr) {
  REAL_FUNC_PASSTHROUGH_VOID ( pthread_exit ) ( value_ptr );
}

LIB_PRIVATE
int _real_pthread_detach(pthread_t thread)
{
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_detach ) ( thread );
}

LIB_PRIVATE
int _real_pthread_kill(pthread_t thread, int sig)
{
  REAL_FUNC_PASSTHROUGH_TYPED ( int,pthread_kill )
    ( thread, sig );
}

LIB_PRIVATE
int _real_access(const char *pathname, int mode)
{
  REAL_FUNC_PASSTHROUGH_TYPED ( int,access ) ( pathname,mode );
}

LIB_PRIVATE
struct dirent *_real_readdir(DIR *dirp) {
  REAL_FUNC_PASSTHROUGH_TYPED ( struct dirent *, readdir ) ( dirp );
}

LIB_PRIVATE
int _real_readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result ) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int, readdir_r ) ( dirp, entry, result );
}

LIB_PRIVATE
int _real_rand(void) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,rand ) ( );
}

LIB_PRIVATE
void _real_srand(unsigned int seed) {
  REAL_FUNC_PASSTHROUGH_VOID ( srand ) ( seed );
}

LIB_PRIVATE
time_t _real_time(time_t *tloc) {
  REAL_FUNC_PASSTHROUGH_TYPED ( time_t,time ) ( tloc );
}

LIB_PRIVATE
FILE * _real_tmpfile(void) {
  REAL_FUNC_PASSTHROUGH_TYPED ( FILE *,tmpfile ) ( );
}

LIB_PRIVATE
int _real_truncate(const char *path, off_t length) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,truncate ) ( path, length );
}

LIB_PRIVATE
int _real_ftruncate(int fd, off_t length) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,ftruncate ) ( fd, length );
}

LIB_PRIVATE
int _real_truncate64(const char *path, off64_t length) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,truncate64 ) ( path, length );
}

LIB_PRIVATE
int _real_ftruncate64(int fd, off64_t length) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,ftruncate64 ) ( fd, length );
}

LIB_PRIVATE
int _real_dup(int oldfd) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,dup ) ( oldfd );
}

LIB_PRIVATE
int _real_dup2(int oldfd, int newfd) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,dup2 ) ( oldfd, newfd );
}

LIB_PRIVATE
int _real_dup3(int oldfd, int newfd, int flags) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,dup3 ) ( oldfd, newfd, flags );
}

LIB_PRIVATE
off_t _real_lseek(int fd, off_t offset, int whence) {
  REAL_FUNC_PASSTHROUGH_TYPED ( off_t,lseek) ( fd, offset, whence );
}

LIB_PRIVATE
off64_t _real_lseek64(int fd, off64_t offset, int whence) {
  REAL_FUNC_PASSTHROUGH_TYPED ( off64_t,lseek64) ( fd, offset, whence );
}

LIB_PRIVATE
loff_t _real_llseek(int fd, loff_t offset, int whence) {
  REAL_FUNC_PASSTHROUGH_TYPED ( loff_t,llseek) ( fd, offset, whence );
}

LIB_PRIVATE
int _real_unlink(const char *pathname) {
  REAL_FUNC_PASSTHROUGH_TYPED ( int,unlink ) ( pathname );
}

LIB_PRIVATE
ssize_t _real_pread(int fd, void *buf, size_t count, off_t offset) {
  REAL_FUNC_PASSTHROUGH_TYPED ( ssize_t,pread ) ( fd, buf, count, offset );
}

LIB_PRIVATE
ssize_t _real_readv(int fd, const struct iovec *iov, int iovcnt) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, readv) (fd, iov, iovcnt);
}

LIB_PRIVATE
ssize_t _real_writev(int fd, const struct iovec *iov, int iovcnt) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, writev) (fd, iov, iovcnt);
}

LIB_PRIVATE
ssize_t _real_preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, preadv) (fd, iov, iovcnt, offset);
}

LIB_PRIVATE
ssize_t _real_pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, pwritev) (fd, iov, iovcnt, offset);
}

LIB_PRIVATE
ssize_t _real_pwrite(int fd, const void *buf, size_t count, off_t offset) {
  REAL_FUNC_PASSTHROUGH_TYPED ( ssize_t,pwrite ) ( fd, buf, count, offset );
}

LIB_PRIVATE
int _real_getpwnam_r(const char *name, struct passwd *pwd,
                     char *buf, size_t buflen, struct passwd **result) {
  REAL_FUNC_PASSTHROUGH (getpwnam_r) (name, pwd, buf, buflen, result);
}

LIB_PRIVATE
int _real_getpwuid_r(uid_t uid, struct passwd *pwd,
                     char *buf, size_t buflen, struct passwd **result) {
  REAL_FUNC_PASSTHROUGH (getpwuid_r) (uid, pwd, buf, buflen, result);
}

LIB_PRIVATE
int _real_getgrnam_r(const char *name, struct group *grp,
                     char *buf, size_t buflen, struct group **result) {
  REAL_FUNC_PASSTHROUGH (getgrnam_r) (name, grp, buf, buflen, result);
}

LIB_PRIVATE
int _real_getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen,
                     struct group **result) {
  REAL_FUNC_PASSTHROUGH (getgrgid_r) (gid, grp, buf, buflen, result);
}

LIB_PRIVATE
int _real_getaddrinfo(const char *node, const char *service,
                      const struct addrinfo *hints, struct addrinfo **res) {
  REAL_FUNC_PASSTHROUGH (getaddrinfo) (node, service, hints, res);
}
LIB_PRIVATE
void _real_freeaddrinfo(struct addrinfo *res) {
  REAL_FUNC_PASSTHROUGH_VOID (freeaddrinfo) (res);
}

LIB_PRIVATE
int _real_getnameinfo(const struct sockaddr *sa, socklen_t salen,
                      char *host, size_t hostlen,
                      char *serv, size_t servlen, int flags) {
  REAL_FUNC_PASSTHROUGH (getnameinfo) (sa, salen, host, hostlen, serv, servlen,
                                       flags);
}

LIB_PRIVATE
ssize_t _real_sendto(int sockfd, const void *buf, size_t len, int flags,
                     const struct sockaddr *dest_addr, socklen_t addrlen) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, sendto) (sockfd, buf, len, flags,
                                                 dest_addr, addrlen);
}

LIB_PRIVATE
ssize_t _real_sendmsg(int sockfd, const struct msghdr *msg, int flags) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, sendmsg) (sockfd, msg, flags);
}

LIB_PRIVATE
ssize_t _real_recvfrom(int sockfd, void *buf, size_t len, int flags,
                       struct sockaddr *src_addr, socklen_t *addrlen) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, recvfrom) (sockfd, buf, len, flags,
                                                   src_addr, addrlen);
}

LIB_PRIVATE
ssize_t _real_recvmsg(int sockfd, struct msghdr *msg, int flags) {
  REAL_FUNC_PASSTHROUGH_TYPED (ssize_t, recvmsg) (sockfd, msg, flags);
}


LIB_PRIVATE
void _real_flockfile(FILE *filehandle) {
  REAL_FUNC_PASSTHROUGH_VOID (flockfile) (filehandle);
}

LIB_PRIVATE
int  _real_ftrylockfile(FILE *filehandle) {
  REAL_FUNC_PASSTHROUGH (ftrylockfile) (filehandle);
}

LIB_PRIVATE
void _real_funlockfile(FILE *filehandle) {
  REAL_FUNC_PASSTHROUGH_VOID (funlockfile) (filehandle);
}

/* See comments for syscall wrapper */
LIB_PRIVATE
long int _real_syscall(long int sys_num, ... ) {
  int i;
  void * arg[7];
  va_list ap;

  va_start(ap, sys_num);
  for (i = 0; i < 7; i++)
    arg[i] = va_arg(ap, void *);
  va_end(ap);

  // /usr/include/unistd.h says syscall returns long int (contrary to man page)
  REAL_FUNC_PASSTHROUGH_TYPED ( long int, syscall ) ( sys_num, arg[0], arg[1],
                                                      arg[2], arg[3], arg[4],
                                                      arg[5], arg[6] );
}

LIB_PRIVATE
int _real_epoll_create(int size) {
  REAL_FUNC_PASSTHROUGH (epoll_create) (size);
}

LIB_PRIVATE
int _real_epoll_create1(int flags) {
  REAL_FUNC_PASSTHROUGH (epoll_create1) (flags);
}

LIB_PRIVATE
int _real_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
  REAL_FUNC_PASSTHROUGH (epoll_ctl) (epfd, op, fd, event);
}

LIB_PRIVATE
int _real_epoll_wait(int epfd, struct epoll_event *events,
                     int maxevents, int timeout) {
  REAL_FUNC_PASSTHROUGH (epoll_wait) (epfd, events, maxevents, timeout);
}

//LIB_PRIVATE
//int _real_epoll_pwait(int epfd, struct epoll_event *events,
//                      int maxevents, int timeout, const sigset_t *sigmask) {
//  REAL_FUNC_PASSTHROUGH (epoll_pwait) (epfd, events, maxevents, timeout, sigmask);
//}
