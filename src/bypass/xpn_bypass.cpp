
/*
 *  Copyright 2020-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
 *
 *  This file is part of Expand.
 *
 *  Expand is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Expand is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Expand.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


/* ... Include / Inclusion ........................................... */

#include "xpn_bypass.h"
#include <signal.h>


#ifdef __cplusplus
extern "C"
{
#endif

/* ... Const / Const ................................................. */


/* ... Global variables / Variables globales ........................ */

/**
 * This variable indicates if expand has already been initialized or not.
 *   0 indicates that expand has NOT been initialized yet.
 *   1 indicates that expand has already been initialized.
 */
static int xpn_adaptor_initCalled = 0;
static int xpn_adaptor_initCalled_getenv = 0; // env variable obtained

/**
 * This variable contains the prefix which will be considerated as expand partition.
 */
const char *xpn_adaptor_partition_prefix = "/tmp/expand/"; // Original --> xpn://
int   xpn_prefix_change_verified = 0;
size_t   xpn_prefix_strlen = 12;


/* ... Auxiliar functions / Funciones auxiliares ......................................... */

/**
 * Check that the path contains the prefix of XPN
 */
int is_xpn_prefix   ( const char * path )
{
  if (0 == xpn_prefix_change_verified)
  {
    xpn_prefix_change_verified = 1;

    char * env_prefix = getenv("XPN_MOUNT_POINT");
    if (env_prefix != NULL)
    {
      xpn_adaptor_partition_prefix = env_prefix;
    }
  }

  return ( strlen(path) > xpn_prefix_strlen && !memcmp(xpn_adaptor_partition_prefix, path, xpn_prefix_strlen) );
}

/**
 * Skip the XPN prefix
 */
const char * skip_xpn_prefix ( const char * path )
{
  return (const char *)(path + xpn_prefix_strlen);
}


/**
 * File descriptors table management
 */
struct generic_fd * fdstable = NULL;
long   fdstable_size = 0L;
long   fdstable_first_free = 0L;

void fdstable_realloc ( void )
{
  long old_size = fdstable_size;
  struct generic_fd * fdstable_aux = fdstable;

  debug_info("[BYPASS] >> Begin fdstable_realloc....");

  if ( NULL == fdstable )
  {
    fdstable_size = (long) MAX_FDS;
    fdstable = (struct generic_fd *) malloc(fdstable_size * sizeof(struct generic_fd));
  }
  else
  {
    fdstable_size = fdstable_size * 2;
    fdstable = (struct generic_fd *) realloc((struct generic_fd *)fdstable, fdstable_size * sizeof(struct generic_fd));
  }

  if ( NULL == fdstable )
  {
    debug_error("Error: out of memory");
    if (fdstable_aux != NULL) {
      free(fdstable_aux);
    }

    exit(-1);
  }
  
  for (int i = old_size; i < fdstable_size; ++i)
  {
    fdstable[i].type = FD_FREE;
    fdstable[i].real_fd = -1;
    // fdstable[i].is_file = -1;
  }

  debug_info("[BYPASS] << After fdstable_realloc....");
}

void fdstable_init ( void )
{
  debug_info("[BYPASS] >> Begin fdstable_init....");

  fdstable_realloc();

  debug_info("[BYPASS] << After fdstable_init....");
}

struct generic_fd fdstable_get ( int fd )
{
  struct generic_fd ret;
  
  debug_info("[BYPASS] >> Begin fdstable_get....");
  debug_info("[BYPASS]    1) fd  => "<<fd);

  if ((NULL != fdstable) && (fd >= PLUSXPN))
  {
    fd = fd - PLUSXPN;
    ret = fdstable[fd];
  }
  else
  {
    ret.type = FD_SYS;
    ret.real_fd = fd;
  }

  debug_info("[BYPASS]\t fdstable_get -> type: "<<ret.type<<" ; real_fd: "<<ret.real_fd);
  debug_info("[BYPASS] << After fdstable_get....");

  return ret;
}

int fdstable_put ( struct generic_fd fd )
{
  debug_info("[BYPASS] >> Begin fdstable_put....");

  for (int i = fdstable_first_free; i < fdstable_size; ++i)
  {
    if ( fdstable[i].type == FD_FREE ) {
      fdstable[i] = fd;
      fdstable_first_free = (long)(i + 1);

      debug_info("[BYPASS]\t fdstable_put -> fd "<<i + PLUSXPN<<" ; type: "<<fdstable[i].type<<" ; real_fd: "<<fdstable[i].real_fd);
      debug_info("[BYPASS] << After fdstable_put....");

      return i + PLUSXPN;
    }
  }

  long old_size = fdstable_size;

  fdstable_realloc();

  if ( fdstable[old_size].type == FD_FREE ) {
    fdstable[old_size] = fd;

    debug_info("[BYPASS]\t fdstable_put -> fd "<<old_size + PLUSXPN<<" ; type: "<<fdstable[old_size].type<<" ; real_fd: "<<fdstable[old_size].real_fd);
    debug_info("[BYPASS] << After fdstable_put....");

    return old_size + PLUSXPN;
  }

  debug_info("[BYPASS]\t fdstable_put -> -1");
  debug_info("[BYPASS] << After fdstable_put....");

  return -1;
}

int fdstable_remove ( int fd )
{
  debug_info("[BYPASS] >> Begin fdstable_remove....");
  debug_info("[BYPASS]    1) fd  => "<<fd);

  if (fd < PLUSXPN) {
    debug_info("[BYPASS] << After fdstable_remove....");

    return 0;
  }

  fd = fd - PLUSXPN;
  fdstable[fd].type    = FD_FREE;
  fdstable[fd].real_fd = -1;
  // fdstable[fd].is_file = -1;

  if (fd < fdstable_first_free) {
    fdstable_first_free = fd;
  }

  debug_info("[BYPASS] << After fdstable_remove....");

  return 0;
}

int add_xpn_file_to_fdstable ( int fd )
{
  // struct stat st;
  struct generic_fd virtual_fd;
  
  debug_info("[BYPASS] >> Begin add_xpn_file_to_fdstable....");
  debug_info("[BYPASS]    1) fd  => "<<fd);

  int ret = fd;

  // check arguments
  if (fd < 0) {
    debug_info("[BYPASS]\t add_xpn_file_to_fdstable -> "<<ret);
    debug_info("[BYPASS] << After add_xpn_file_to_fdstable....");

    return ret;
  } 

  // fstat(fd...
  // xpn_fstat(fd, &st);

  // setup virtual_fd
  virtual_fd.type    = FD_XPN;
  virtual_fd.real_fd = fd;
  // virtual_fd.is_file = (S_ISDIR(st.st_mode)) ? 0 : 1;

  // insert into fdstable
  ret = fdstable_put ( virtual_fd );

  debug_info("[BYPASS]\t add_xpn_file_to_fdstable -> "<<ret);
  debug_info("[BYPASS] << After add_xpn_file_to_fdstable....");

  return ret;
}


/**
 * Dir table management
 */
DIR ** fdsdirtable = NULL;
long   fdsdirtable_size = 0L;
long   fdsdirtable_first_free = 0L;

void fdsdirtable_realloc ( void )
{
  long          old_size = fdsdirtable_size;
  DIR ** fdsdirtable_aux = fdsdirtable;
  
  debug_info("[BYPASS] >> Begin fdsdirtable_realloc....");
  
  if ( NULL == fdsdirtable )
  {
    fdsdirtable_size = (long) MAX_DIRS;
    fdsdirtable = (DIR **) malloc(MAX_DIRS * sizeof(DIR *));
  }
  else
  {
    fdsdirtable_size = fdsdirtable_size * 2;
    fdsdirtable = (DIR **) realloc((DIR **)fdsdirtable, fdsdirtable_size * sizeof(DIR *));
  }

  if ( NULL == fdsdirtable )
  {
    debug_error("Error: out of memory");
    if (NULL != fdsdirtable_aux) {
      free(fdsdirtable_aux);
    }

    exit(-1);
  }
  
  for (int i = old_size; i < fdsdirtable_size; ++i) {
    fdsdirtable[i] = NULL;
  }

  debug_info("[BYPASS] << After fdsdirtable_realloc....");
}

void fdsdirtable_init ( void )
{
  debug_info("[BYPASS] >> Begin fdsdirtable_init....");

  fdsdirtable_realloc();

  debug_info("[BYPASS] << After fdsdirtable_init....");
}

int fdsdirtable_get ( DIR * dir )
{
  debug_info("[BYPASS] >> Begin fdsdirtable_get....");
  debug_info("[BYPASS]    1) dir  => "<<dir);

  for (int i = 0; i < fdsdirtable_size; ++i)
  {
    if ( fdsdirtable[i] == dir ) {
      debug_info("[BYPASS]\t fdsdirtable_get -> "<<i);
      debug_info("[BYPASS] << After fdsdirtable_get....");

      return i;
    }
  }

  debug_info("[BYPASS]\t fdsdirtable_get -> -1");
  debug_info("[BYPASS] << After fdsdirtable_get....");

  return -1;
}

int fdsdirtable_put ( DIR * dir )
{
  // preparing the "file side" of the directory
  struct generic_fd virtual_fd;
  int fd;
  int vfd;
  
  debug_info("[BYPASS] >> Begin fdsdirtable_put....");
  debug_info("[BYPASS]    1) dir  => "<<dir);

  fd = dirfd(dir);

  virtual_fd.type    = FD_XPN;
  virtual_fd.real_fd = fd;
  // virtual_fd.is_file = 0;

  // insert into the dirtable (and fdstable)
  for (int i = fdsdirtable_first_free; i < fdsdirtable_size; ++i)
  {
    if (fdsdirtable[i] == NULL)
    {
      fdsdirtable[i] = dir;
      fdsdirtable_first_free = (long)(i + 1);

      vfd = fdstable_put ( virtual_fd );
      dir->fd = vfd;

      debug_info("[BYPASS]\t fdsdirtable_put -> 0");
      debug_info("[BYPASS] << After fdsdirtable_put....");

      return 0;
    }
  }

  long old_size = fdstable_size;
  fdsdirtable_realloc();

  if (fdsdirtable[old_size] == NULL)
  {
    fdsdirtable[old_size] = dir;
    fdsdirtable_first_free = (long)(old_size + 1);

    vfd = fdstable_put ( virtual_fd );
    dir->fd = vfd;

    debug_info("[BYPASS]\t fdsdirtable_put -> 0");
    debug_info("[BYPASS] << After fdsdirtable_put....");

    return 0;
  }

  debug_info("[BYPASS]\t fdsdirtable_put -> -1");
  debug_info("[BYPASS] << After fdsdirtable_put....");

  return -1;
}

int fdsdirtable_remove ( DIR * dir )
{
  debug_info("[BYPASS] >> Begin fdsdirtable_remove....");
  debug_info("[BYPASS]    1) dir  => "<<dir);

  for (int i = 0; i < fdsdirtable_size; ++i)
  {
    if (fdsdirtable[i] == dir)
    {
      fdstable_remove ( dir->fd );
      dir->fd = dir->fd - PLUSXPN;

      fdsdirtable[i] = NULL;

      if (i < fdsdirtable_first_free) {
        fdsdirtable_first_free = i;
      }

      debug_info("[BYPASS]\t fdsdirtable_remove -> 0");
      debug_info("[BYPASS] << After fdsdirtable_remove....");

      return 0;
    }
  }

  debug_info("[BYPASS]\t fdsdirtable_remove -> -1");
  debug_info("[BYPASS] << After fdsdirtable_remove....");

  return -1;
}

DIR fdsdirtable_getfd ( DIR * dir )
{
  DIR aux_dirp;
  
  debug_info("[BYPASS] >> Begin fdsdirtable_getfd....");
  debug_info("[BYPASS]    1) dir  => "<<dir);

  aux_dirp = *dir;
  
  struct generic_fd virtual_fd = fdstable_get ( aux_dirp.fd );
  aux_dirp.fd = virtual_fd.real_fd;

  debug_info("[BYPASS] << After fdsdirtable_getfd....");

  return aux_dirp;
}


/**
 * stat management
 */
int stat_to_stat64 ( struct stat64 *buf, struct stat *st )
{
  buf->st_dev     = (__dev_t)      st->st_dev;
  buf->st_ino     = (__ino64_t)    st->st_ino;
  buf->st_mode    = (__mode_t)     st->st_mode;
  buf->st_nlink   = (__nlink_t)    st->st_nlink;
  buf->st_uid     = (__uid_t)      st->st_uid;
  buf->st_gid     = (__gid_t)      st->st_gid;
  buf->st_rdev    = (__dev_t)      st->st_rdev;
  buf->st_size    = (__off64_t)    st->st_size;
  buf->st_blksize = (__blksize_t)  st->st_blksize;
  buf->st_blocks  = (__blkcnt64_t) st->st_blocks;
  buf->st_atime   = (__time_t)     st->st_atime;
  buf->st_mtime   = (__time_t)     st->st_mtime;
  buf->st_ctime   = (__time_t)     st->st_ctime;

  return 0;
}

int stat64_to_stat ( struct stat *buf, struct stat64 *st )
{
  buf->st_dev     = (__dev_t)     st->st_dev;
  buf->st_ino     = (__ino_t)     st->st_ino;
  buf->st_mode    = (__mode_t)    st->st_mode;
  buf->st_nlink   = (__nlink_t)   st->st_nlink;
  buf->st_uid     = (__uid_t)     st->st_uid;
  buf->st_gid     = (__gid_t)     st->st_gid;
  buf->st_rdev    = (__dev_t)     st->st_rdev;
  buf->st_size    = (__off_t)     st->st_size;
  buf->st_blksize = (__blksize_t) st->st_blksize;
  buf->st_blocks  = (__blkcnt_t)  st->st_blocks;
  buf->st_atime   = (__time_t)    st->st_atime;
  buf->st_mtime   = (__time_t)    st->st_mtime;
  buf->st_ctime   = (__time_t)    st->st_ctime;

  return 0;
}


/*
 * This function checks if expand has already been initialized.
 * If not, it initialize it.
 */
int xpn_adaptor_keepInit ( void )
{
  int    ret;
  char * xpn_adaptor_initCalled_env = NULL;
  
  debug_info("[BYPASS] >> Begin xpn_adaptor_keepInit....");

  if (0 == xpn_adaptor_initCalled_getenv)
  {
    xpn_adaptor_initCalled_env = getenv("INITCALLED");
    xpn_adaptor_initCalled     = 0;

    if (xpn_adaptor_initCalled_env != NULL) {
      xpn_adaptor_initCalled = atoi(xpn_adaptor_initCalled_env);
    }

    xpn_adaptor_initCalled_getenv = 1;
  }
  
  ret = 0;

  // If expand has not been initialized, then initialize it.
  if (0 == xpn_adaptor_initCalled)
  {
    xpn_adaptor_initCalled = 1; //TODO: Delete
    setenv("INITCALLED", "1", 1);

    debug_info("[BYPASS]\t Begin xpn_init()");

    fdstable_init ();
    fdsdirtable_init ();
    ret = xpn_init();
    // Add callback atexit of xpn_destroy
    atexit((void (*)(void))xpn_destroy);

    debug_info("[BYPASS]\t After xpn_init() -> "<<ret);

    if (ret < 0)
    {
      debug_error("ERROR: Expand xpn_init couldn't be initialized :-(");
      xpn_adaptor_initCalled = 0;
      setenv("INITCALLED", "0", 1);
    }
    else
    {
      xpn_adaptor_initCalled = 1;
      setenv("INITCALLED", "1", 1);
    }
  }

  debug_info("[BYPASS]\t xpn_adaptor_keepInit -> "<<ret);
  debug_info("[BYPASS] << After xpn_adaptor_keepInit....");

  return ret;
}


/* ... Functions / Funciones ......................................... */

// File API
int open ( const char *path, int flags, ... )
{
  int ret, fd;
  va_list ap;
  mode_t mode = 0;

  va_start(ap, flags);

  mode = va_arg(ap, mode_t);

  debug_info("[BYPASS] >> Begin open....");
  debug_info("[BYPASS]    1) Path  => "<<path);
  debug_info("[BYPASS]    2) Flags => "<<flags);
  debug_info("[BYPASS]    3) Mode  => "<<mode);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_open ("<<(path+xpn_prefix_strlen)<<", "<<flags<<")");

    if (mode != 0) {
      fd = xpn_open(skip_xpn_prefix(path), flags, mode);
    }
    else {
      fd = xpn_open(skip_xpn_prefix(path), flags);
    }

    debug_info("[BYPASS]\t xpn_open ("<<skip_xpn_prefix(path)<<","<<flags<<") -> "<<fd);

    ret = add_xpn_file_to_fdstable(fd);
  }
  // Not an XPN partition. We must link with the standard library.
  else 
  {
    debug_info("[BYPASS]\t PROXY(open) ("<<path<<","<<flags<<","<<mode<<")");

    ret = PROXY(open)((char *)path, flags, mode);

    debug_info("[BYPASS]\t PROXY(open) ("<<path<<","<<flags<<","<<mode<<") -> "<<ret);
  }

  va_end(ap);

  debug_info("[BYPASS] << After open....");

  return ret;
}


int open64 ( const char *path, int flags, ... )
{
  int fd, ret;
  va_list ap;
  mode_t mode = 0;

  va_start(ap, flags);

  mode = va_arg(ap, mode_t);

  debug_info("[BYPASS] >> Begin open64....");
  debug_info("[BYPASS]    1) Path  => "<<path);
  debug_info("[BYPASS]    2) flags => "<<flags);
  debug_info("[BYPASS]    3) mode  => "<<mode);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_open ("<<(path + xpn_prefix_strlen)<<","<<flags<<")");

    if (mode != 0) {
      fd = xpn_open(skip_xpn_prefix(path), flags, mode);
    }
    else {
      fd = xpn_open(skip_xpn_prefix(path), flags);
    }

    debug_info("[BYPASS]\t xpn_open ("<<skip_xpn_prefix(path)<<","<<flags<<") -> "<<fd);

    ret = add_xpn_file_to_fdstable(fd);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t PROXY(open64) ("<<path<<","<<flags<<","<<mode<<")");

    ret = PROXY(open64)((char *)path, flags, mode);

    debug_info("[BYPASS]\t PROXY(open64) ("<<path<<","<<flags<<","<<mode<<") -> "<<ret);
  }

  va_end(ap);

  debug_info("[BYPASS] << After open64....");

  return ret;
}


#ifndef HAVE_ICC

int __open_2 ( const char *path, int flags, ... )
{
  int fd, ret;
  va_list ap;
  mode_t mode = 0;

  va_start(ap, flags);
  mode = va_arg(ap, mode_t);

  debug_info("[BYPASS] >> Begin __open_2....");
  debug_info("[BYPASS]    1) Path  => "<<path);
  debug_info("[BYPASS]    2) flags => "<<flags);
  debug_info("[BYPASS]    3) mode  => "<<mode);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_open ("<<(path + xpn_prefix_strlen)<<","<<flags<<")");

    if (mode != 0) {
      fd=xpn_open(skip_xpn_prefix(path), flags, mode);
    }
    else {
      fd=xpn_open(skip_xpn_prefix(path), flags);
    }

    debug_info("[BYPASS]\t xpn_open ("<<skip_xpn_prefix(path)<<","<<flags<<") -> "<<fd);

    ret = add_xpn_file_to_fdstable(fd);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__open_2) "<<path);

    ret = PROXY(__open_2)((char *)path, flags);

    debug_info("[BYPASS]\t PROXY(__open_2) "<<path<<" -> "<<ret);
  }

  va_end(ap);

  debug_info("[BYPASS] << After __open_2....");

  return ret;
}

#endif

int creat ( const char *path, mode_t mode )
{
  int fd,ret;

  debug_info("[BYPASS] >> Begin creat....");

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t try to creat "<<skip_xpn_prefix(path));

    fd  = xpn_creat((const char *)skip_xpn_prefix(path),mode);
    ret = add_xpn_file_to_fdstable(fd);

    debug_info("[BYPASS]\t creat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(creat) "<<path);

    ret = PROXY(creat)(path, mode);

    debug_info("[BYPASS]\t PROXY(creat) "<<path<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After creat....");
  return ret;
}

int mkstemp (char *templ)
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin mkstemp...");
  debug_info("[BYPASS]    1) template "<<templ);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(templ))
  {
    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t try to creat "<<skip_xpn_prefix(templ));

    srand(time(NULL));
    int n = rand()%100000;

    char *str_init = strstr(templ, "XXXXXX");
    sprintf(str_init,"%06d", n);

    int fd  = xpn_creat((const char *)skip_xpn_prefix(templ), S_IRUSR | S_IWUSR);
    ret = add_xpn_file_to_fdstable(fd);

    debug_info("[BYPASS]\t creat "<<skip_xpn_prefix(templ)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(mkstemp)");

    ret = PROXY(mkstemp)(templ);

    debug_info("[BYPASS]\t PROXY(mkstemp) -> "<<ret);
  }

  debug_info("[BYPASS] << After mkstemp...");

  return ret;
}

int ftruncate ( int fd, off_t length )
{
  debug_info("[BYPASS] >> Begin ftruncate...");

  int ret = -1;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t try to xpn_ftruncate");

    ret = xpn_ftruncate(virtual_fd.real_fd, length);

    debug_info("[BYPASS]\t xpn_ftruncate -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(ftruncate) "<<fd<<", "<<length);

    ret = PROXY(ftruncate)(fd, length);

    debug_info("[BYPASS]\t try to PROXY(ftruncate) "<<fd<<", "<<length<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After ftruncate...");

  return ret;
}

ssize_t read ( int fd, void *buf, size_t nbyte )
{         
  ssize_t ret = -1;

  debug_info("[BYPASS] >> Begin read...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * buf="<<buf);
  debug_info("[BYPASS]    * byte="<<nbyte);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0)
    // {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After read...");

    //   errno = EISDIR;
    //   return -1;
    // }

    debug_info("[BYPASS]\t try to xpn_read "<<virtual_fd.real_fd<<", "<<buf<<", "<<nbyte);

    ret = xpn_read(virtual_fd.real_fd, buf, nbyte);

    debug_info("[BYPASS]\t try to xpn_read "<<virtual_fd.real_fd<<", "<<buf<<", "<<nbyte<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(read) "<<fd<<" ,"<<buf<<" ,"<<nbyte);

    ret = PROXY(read)(fd, buf, nbyte);

    debug_info("[BYPASS]\t try to PROXY(read) "<<fd<<" ,"<<buf<<" ,"<<nbyte<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After read...");

  return ret;
}

ssize_t write ( int fd, const void *buf, size_t nbyte )
{
  ssize_t ret = -1;

  debug_info("[BYPASS] >> Begin write...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * buf="<<buf);
  debug_info("[BYPASS]    * byte="<<nbyte);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0)
    // {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After write...");

    //   errno = EISDIR;
    //   return -1;
    // }

    debug_info("[BYPASS]\t try to xpn_write "<<virtual_fd.real_fd<<", "<<buf<<", "<<nbyte);

    ret = xpn_write(virtual_fd.real_fd, (void *)buf, nbyte);

    debug_info("[BYPASS]\t xpn_write "<<virtual_fd.real_fd<<", "<<buf<<", "<<nbyte<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(write) "<<fd<<" ,"<<buf<<" ,"<<nbyte);

    ret = PROXY(write)(fd, (void *)buf, nbyte);

    debug_info("[BYPASS]\t try to PROXY(write) "<<fd<<" ,"<<buf<<" ,"<<nbyte<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After write...");

  return ret;
}

ssize_t pread ( int fd, void *buf, size_t count, off_t offset )
{
  ssize_t ret = -1;

  debug_info("[BYPASS] >> Begin pread...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * buf="<<buf);
  debug_info("[BYPASS]    * count="<<count);
  debug_info("[BYPASS]    * offset="<<offset);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0)
    // {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After pread...");

    //   errno = EISDIR;
    //   return -1;
    // }

    debug_info("[BYPASS]\t try to xpn_read "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<", "<<offset);

    ret = xpn_lseek(virtual_fd.real_fd, offset, SEEK_SET);
    if (ret != -1) {
      ret = xpn_read(virtual_fd.real_fd, buf, count);
    }
    if (ret != -1) {
      xpn_lseek(virtual_fd.real_fd, -ret, SEEK_CUR);
    }

    debug_info("[BYPASS]\t xpn_read "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(pread) "<<fd<<", "<<buf<<", "<<count);

    ret = PROXY(pread)(fd,buf, count, offset);

    debug_info("[BYPASS]\t PROXY(pread) "<<fd<<", "<<buf<<", "<<count<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After pread...");

  return ret;
}

ssize_t pwrite ( int fd, const void *buf, size_t count, off_t offset )
{
  ssize_t ret = -1;

  debug_info("[BYPASS] >> Begin pwrite...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * buf="<<buf);
  debug_info("[BYPASS]    * count="<<count);
  debug_info("[BYPASS]    * offset="<<offset);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0)
    // {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After pwrite...");

    //   errno = EISDIR;
    //   return -1;
    // }

    debug_info("[BYPASS]\t try to xpn_write "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<", "<<offset);

    ret = xpn_lseek(virtual_fd.real_fd, offset, SEEK_SET);
    if (ret != -1) {
      ret = xpn_write(virtual_fd.real_fd, buf, count);
    }
    if (ret != -1) {
      xpn_lseek(virtual_fd.real_fd, -ret, SEEK_CUR);
    }

    debug_info("[BYPASS]\t xpn_write "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(pwrite) "<<fd<<", "<<buf<<", "<<count<<", "<<offset);

    ret = PROXY(pwrite)(fd, buf, count, offset);

    debug_info("[BYPASS]\t PROXY(pwrite) "<<fd<<", "<<buf<<", "<<count<<", "<<offset<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After pwrite...");

  return ret;
}

ssize_t pread64 ( int fd, void *buf, size_t count, off_t offset )
{
  ssize_t ret = -1;

  debug_info("[BYPASS] >> Begin pread64...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * buf="<<buf);
  debug_info("[BYPASS]    * count="<<count);
  debug_info("[BYPASS]    * offset="<<offset);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0)
    // {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After pread64...");

    //   errno = EISDIR;
    //   return -1;
    // }

    debug_info("[BYPASS]\t try to xpn_read "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<", "<<offset);

    ret = xpn_lseek(virtual_fd.real_fd, offset, SEEK_SET);
    if (ret != -1) {
      ret = xpn_read(virtual_fd.real_fd, buf, count);
    }
    if (ret != -1) {
      xpn_lseek(virtual_fd.real_fd, -ret, SEEK_CUR);
    }

    debug_info("[BYPASS]\t xpn_read "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(pread64) "<<fd<<", "<<buf<<", "<<count);

    ret = PROXY(pread64)(fd,buf, count, offset);

    debug_info("[BYPASS]\t PROXY(pread64) "<<fd<<", "<<buf<<", "<<count<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After pread64...");

  return ret;
}

ssize_t pwrite64 ( int fd, const void *buf, size_t count, off_t offset )
{
  ssize_t ret = -1;

  debug_info("[BYPASS] >> Begin pwrite64...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * buf="<<buf);
  debug_info("[BYPASS]    * count="<<count);
  debug_info("[BYPASS]    * offset="<<offset);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0)
    // {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After pwrite64...");

    //   errno = EISDIR;
    //   return -1;
    // }

    debug_info("[BYPASS]\t try to xpn_write "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<", "<<offset);

    ret = xpn_lseek(virtual_fd.real_fd, offset, SEEK_SET);
    if (ret != -1) {
      ret = xpn_write(virtual_fd.real_fd, buf, count);
    }
    if (ret != -1) {
      xpn_lseek(virtual_fd.real_fd, -ret, SEEK_CUR);
    }

    debug_info("[BYPASS]\t xpn_write "<<virtual_fd.real_fd<<", "<<buf<<", "<<count<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(pwrite64) "<<fd<<", "<<buf<<", "<<count<<", "<<offset);

    ret = PROXY(pwrite64)(fd, buf, count, offset);

    debug_info("[BYPASS]\t PROXY(pwrite64) "<<fd<<", "<<buf<<", "<<count<<", "<<offset<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After pwrite64...");

  return ret;
}

off_t lseek ( int fd, off_t offset, int whence )
{
  off_t ret = (off_t) -1;

  debug_info("[BYPASS] >> Begin lseek...");

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_lseek "<<fd<<", "<<offset<<", "<<whence);

    ret = xpn_lseek(virtual_fd.real_fd, offset, whence);

    debug_info("[BYPASS]\t xpn_lseek "<<fd<<", "<<offset<<", "<<whence<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(lseek) "<<fd<<", "<<offset<<", "<<whence);

    ret = PROXY(lseek)(fd, offset, whence);

    debug_info("[BYPASS]\t PROXY(lseek) "<<fd<<", "<<offset<<", "<<whence<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After lseek...");

  return ret;
}

off64_t lseek64 ( int fd, off64_t offset, int whence )
{
  off64_t ret = (off64_t) -1;

  debug_info("[BYPASS] >> Begin lseek64...");

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_lseek64 "<<fd<<", "<<offset<<", "<<whence);

    ret = xpn_lseek(virtual_fd.real_fd, offset, whence);

    debug_info("[BYPASS]\t xpn_lseek64 "<<fd<<", "<<offset<<", "<<whence<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(lseek64) "<<fd<<", "<<offset<<", "<<whence);

    ret = PROXY(lseek64)(fd, offset, whence);

    debug_info("[BYPASS]\t PROXY(lseek64) "<<fd<<", "<<offset<<", "<<whence<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After lseek64...");

  return ret;
}

int stat(const char *path, struct stat *buf)
{
  int ret;

  debug_info("[BYPASS] >> Begin stat...");
  debug_info("[BYPASS]    1) Path  => "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path));

    ret = xpn_stat(skip_xpn_prefix(path), buf);

    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__xstat)");

    ret = PROXY(__xstat)(_STAT_VER,(const char *)path, buf);

    debug_info("[BYPASS]\t PROXY(__xstat) -> "<<ret);
  }

  debug_info("[BYPASS] << After stat...");

  return ret;
}

int __lxstat64 ( int ver, const char *path, struct stat64 *buf )
{
  int ret;
  struct stat st;

  debug_info("[BYPASS] >> Begin __lxstat64...");
  debug_info("[BYPASS]    1) Ver   => "<<ver);
  debug_info("[BYPASS]    2) Path  => "<<path);
  debug_info("[BYPASS]    3) Buf   => "<<buf);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t try to xpn_stat "<<skip_xpn_prefix(path));

    ret = xpn_stat(skip_xpn_prefix(path), &st);
    if (ret >= 0) {
      stat_to_stat64(buf, &st);
    }

    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__lxstat64)");

    ret = PROXY(__lxstat64)(ver, (const char *)path, buf);

    debug_info("[BYPASS]\t PROXY(__lxstat64) -> "<<ret);
  }

  debug_info("[BYPASS] << After __lxstat64...");

  return ret;
}

int __xstat64 ( int ver, const char *path, struct stat64 *buf )
{
  int ret;
  struct stat st;

  debug_info("[BYPASS] >> Begin __xstat64...");
  debug_info("[BYPASS]    1) Path  => "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix( path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_stat");

    ret = xpn_stat(skip_xpn_prefix(path), &st);
    if (ret >= 0) {
      stat_to_stat64(buf, &st);
    }

    debug_info("[BYPASS]\t xpn_stat -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__xstat64)");

    ret = PROXY(__xstat64)(ver,(const char *)path, buf);

    debug_info("[BYPASS]\t PROXY(__xstat64) -> "<<ret);
  }

  debug_info("[BYPASS] << After __xstat64...");

  return ret;
}

int __fxstat64 ( int ver, int fd, struct stat64 *buf )
{
  int ret;
  struct stat st;

  debug_info("[BYPASS] >> Begin __fxstat64...");
  debug_info("[BYPASS]    1) fd  => "<<fd);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_fstat");

    ret = xpn_fstat(virtual_fd.real_fd, &st);
    if (ret >= 0) {
      stat_to_stat64(buf, &st);
    }

    debug_info("[BYPASS]\t xpn_fstat -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__fxstat64)");

    ret = PROXY(__fxstat64)(ver,fd, buf);

    debug_info("[BYPASS]\t PROXY(__fxstat64) -> "<<ret);
  }

  debug_info("[BYPASS] << After __fxstat64...");

  return ret;
}

int __lxstat ( int ver, const char *path, struct stat *buf )
{
  int ret;

  debug_info("[BYPASS] >> Begin __lxstat...");
  debug_info("[BYPASS]    1) Path  => "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path));

    ret = xpn_stat(skip_xpn_prefix(path), buf);

    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__lxstat)");

    ret = PROXY(__lxstat)(ver,(const char *)path, buf);

    debug_info("[BYPASS]\t PROXY(__lxstat) -> "<<ret);
  }

  debug_info("[BYPASS] << After __lxstat...");

  return ret;
}

int __xstat ( int ver, const char *path, struct stat *buf )
{
  int ret;

  debug_info("[BYPASS] >> Begin __xstat...");
  debug_info("[BYPASS]    1) Path  => "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path));

    ret = xpn_stat(skip_xpn_prefix(path), buf);

    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__xstat)");

    ret = PROXY(__xstat)(ver,(const char *)path, buf);

    debug_info("[BYPASS]\t PROXY(__xstat) -> "<<ret);
  }

  debug_info("[BYPASS] << After __xstat...");

  return ret;
}

int fstat ( int fd, struct stat *buf )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin fstat...");
  debug_info("[BYPASS]    1) fd  => "<<fd);
  debug_info("[BYPASS]    2) buf => "<<buf);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  if (virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_fstat");

    ret = xpn_fstat(virtual_fd.real_fd, buf);

    debug_info("[BYPASS]\t xpn_fstat -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__fxstat)");

    ret = PROXY(__fxstat)(_STAT_VER, fd, buf);

    debug_info("[BYPASS]\t PROXY(__fxstat) -> "<<ret);
  }

  debug_info("[BYPASS] << After fstat...");

  return ret;
}

int __fxstat ( int ver, int fd, struct stat *buf )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin __fxstat...");
  debug_info("[BYPASS]    1) fd  => "<<fd);
  debug_info("[BYPASS]    2) buf => "<<buf);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  if (virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_fstat");

    ret = xpn_fstat(virtual_fd.real_fd, buf);

    debug_info("[BYPASS]\t xpn_fstat -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(__fxstat)");

    ret = PROXY(__fxstat)(ver, fd, buf);

    debug_info("[BYPASS]\t PROXY(__fxstat) -> "<<ret);
  }

  debug_info("[BYPASS] << After __fxstat...");

  return ret;
}

/*
int __fxstatat64 ( __attribute__((__unused__)) int ver, int dirfd, const char *path, struct stat64 *buf, int flags )
{
  int    ret = -1;
  struct stat st;

  debug_info("[BYPASS] >> Begin __fxstatat64...");
  debug_info("[BYPASS]    * ver:   "<<ver);
  debug_info("[BYPASS]    * dirfd: %d\n", dirfd);
  debug_info("[BYPASS]    * path:  "<<path);
  debug_info("[BYPASS]    * buf:   "<<buf);
  debug_info("[BYPASS]    * flags: "<<flags);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix( path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // TODO: if path is relative -> use dirfd as CWD
    // TODO: use flags (see man fstatat

    debug_info("[BYPASS]\t Begin xpn_stat "<<skip_xpn_prefix(path));

    ret = xpn_stat(skip_xpn_prefix(path), &st);
    if (ret >= 0) {
        stat_to_stat64(buf, &st);
    }

    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t PROXY(fstatat64)");

    ret = PROXY(fstatat64)(dirfd, path, buf, flags);

    debug_info("[BYPASS]\t PROXY(fstatat64) -> "<<ret);
  }

  debug_info("[BYPASS] << After __fxstatat64...");

  return ret;
}

int __fxstatat ( __attribute__((__unused__)) int ver, int dirfd, const char *path, struct stat *buf, int flags )
{
  int  ret = -1;

  debug_info("[BYPASS] >> Begin __fxstatat...");
  debug_info("[BYPASS]    * ver:   "<<ver);
  debug_info("[BYPASS]    * path:  "<<path);
  debug_info("[BYPASS]    * dirfd: %d\n", dirfd);
  debug_info("[BYPASS]    * flags: "<<flags);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();
    
    // It is an XPN partition, so we redirect the syscall to expand syscall
    // TODO: if path is relative -> use dirfd as CWD
    // TODO: use flags (see man fstatat

    debug_info("[BYPASS]\t Begin xpn_stat "<<skip_xpn_prefix(path));

    ret = xpn_stat(skip_xpn_prefix(path), buf);

    debug_info("[BYPASS]\t xpn_stat "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t PROXY(fstatat) %d,%p,%p,%d\n", dirfd, path, buf, flags);

    ret = PROXY(fstatat)(dirfd, path, buf, flags);

    debug_info("[BYPASS]\t PROXY(fstatat) %d,%p,%p,%d -> %d\n", dirfd, path, buf, flags, ret);
  }

  debug_info("[BYPASS] << After __fxstatat...");

  return ret;
}
*/

int close ( int fd )
{
  debug_info("[BYPASS] >> Begin close....");
  debug_info("[BYPASS]    * FD = "<<fd);

  int ret = -1;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_close "<<virtual_fd.real_fd);

    ret = xpn_close(virtual_fd.real_fd);
    fdstable_remove(fd);

    debug_info("[BYPASS]\t xpn_close "<<virtual_fd.real_fd<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(close)");

    ret = PROXY(close)(fd);

    debug_info("[BYPASS]\t PROXY(close) -> "<<ret);
  }

  debug_info("[BYPASS] << After close....");

  return ret;
}

int rename ( const char *old_path, const char *new_path )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin rename....");
  debug_info("[BYPASS]    1) old Path "<<old_path);
  debug_info("[BYPASS]    2) new Path "<<new_path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if(is_xpn_prefix( old_path) && is_xpn_prefix( new_path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_rename "<<skip_xpn_prefix(old_path)<<" "<<skip_xpn_prefix(new_path));

    ret = xpn_rename(skip_xpn_prefix(old_path), skip_xpn_prefix(new_path));

    debug_info("[BYPASS]\t xpn_rename "<<skip_xpn_prefix(old_path)<<" "<<skip_xpn_prefix(new_path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else 
  {
    debug_info("[BYPASS]\t try to PROXY(rename) "<<old_path<<", "<<new_path);

    ret = PROXY(rename)(old_path, new_path);

    debug_info("[BYPASS]\t PROXY(rename) "<<old_path<<", "<<new_path<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After rename....");

  return ret;
}

int unlink ( const char *path )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin unlink...");
  debug_info("[BYPASS]    1) Path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_unlink");

    ret = (xpn_unlink(skip_xpn_prefix(path)));

    debug_info("[BYPASS]\t xpn_unlink -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t PROXY(unlink)");

    ret = PROXY(unlink)((char *)path);

    debug_info("[BYPASS]\t PROXY(unlink) -> "<<ret);
  }

  debug_info("[BYPASS] << After unlink....");

  return ret;
}

int remove ( const char *path )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin remove...");
  debug_info("[BYPASS]    1) Path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    struct stat buf;
    ret = xpn_stat(skip_xpn_prefix(path), &buf);

    if ((buf.st_mode & S_IFMT) == S_IFREG) 
    {
      debug_info("[BYPASS]\t xpn_unlink");

      ret = (xpn_unlink(skip_xpn_prefix(path)));

      debug_info("[BYPASS]\t xpn_unlink -> "<<ret);
    }
    else if ((buf.st_mode & S_IFMT) == S_IFDIR)
    {
      debug_info("[BYPASS]\t xpn_rmdir");

      ret = xpn_rmdir( (skip_xpn_prefix(path)) );

      debug_info("[BYPASS]\t xpn_rmdir -> "<<ret);
    }

  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t PROXY(remove)");

    ret = PROXY(remove)((char *)path);

    debug_info("[BYPASS]\t PROXY(remove) -> "<<ret);
  }

  debug_info("[BYPASS] << After remove....");

  return ret;
}

// File API (stdio)
FILE *fopen ( const char *path, const char *mode )
{
  FILE * ret;

  debug_info("[BYPASS] >> Begin fopen....");
  debug_info("[BYPASS]    1) Path  => "<<path);
  debug_info("[BYPASS]    2) Mode  => "<<mode);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_open ("<<(path + xpn_prefix_strlen)<<")");

    int fd;
    switch (mode[0])
    {
      case 'r':
          fd=xpn_open(skip_xpn_prefix(path), O_RDONLY | O_CREAT, 0640);
          break;
      case 'w':
          fd=xpn_open(skip_xpn_prefix(path), O_WRONLY | O_CREAT | O_TRUNC, 0640);
          break;
      default:
          fd=xpn_open(skip_xpn_prefix(path), O_RDWR | O_CREAT | O_TRUNC, 0640);
          break;
    }

    int xpn_fd = add_xpn_file_to_fdstable(fd);

    debug_info("[BYPASS]\t xpn_open ("<<skip_xpn_prefix(path)<<") -> "<<fd);

    debug_info("[BYPASS]\t fdopen "<<xpn_fd);

    ret = fdopen(xpn_fd, mode);

    debug_info("[BYPASS]\t fdopen "<<xpn_fd<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library.
  else 
  {
    debug_info("[BYPASS]\t PROXY(fopen) ("<<path<<","<<mode<<")");

    ret = PROXY(fopen)((const char *)path, mode);

    debug_info("[BYPASS]\t PROXY(fopen) ("<<path<<","<<mode<<") -> "<<ret);
  }

  debug_info("[BYPASS] << After fopen.... "<<path);

  return ret;
}

FILE * fdopen (int fd, const char *mode )
{
  debug_info("[BYPASS] >> Begin fdopen....");
  debug_info("[BYPASS]    1) fd = "<<fd);
  debug_info("[BYPASS]    2) mode = "<<mode);

  FILE *fp;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    debug_info("[BYPASS]\t try to PROXY(fdopen) 1");

    fp = PROXY(fopen)("/dev/null", mode);
    fp->_fileno = fd;

    debug_info("[BYPASS]\t PROXY(fdopen) -> "<<fp);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(fdopen) 2");

    fp = PROXY(fdopen)(fd, mode);

    debug_info("[BYPASS]\t PROXY(fdopen) -> "<<fp);
  }

  debug_info("[BYPASS] << After fdopen....");

  return fp;
}

int fclose ( FILE *stream )
{
  debug_info("[BYPASS] >> Begin fclose....");
  debug_info("[BYPASS]    1) stream = "<<stream);

  int ret = -1;

  int fd = fileno(stream);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_close "<<virtual_fd.real_fd);

    ret = xpn_close(virtual_fd.real_fd);
    fdstable_remove(fd);

    debug_info("[BYPASS]\t xpn_close "<<virtual_fd.real_fd<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to fPROXY(close)");

    ret = PROXY(fclose)(stream);

    debug_info("[BYPASS]\t PROXY(fclose) -> "<<ret);
  }

  debug_info("[BYPASS] << After fclose....");

  return ret;
}

size_t fread ( void *ptr, size_t size, size_t nmemb, FILE *stream )
{         
  size_t ret = (size_t) -1;

  debug_info("[BYPASS] >> Begin fread...");
  debug_info("[BYPASS]    1) ptr="<<ptr);
  debug_info("[BYPASS]    2) size="<<size);
  debug_info("[BYPASS]    3) nmemb="<<nmemb);
  debug_info("[BYPASS]    4) stream="<<stream);

  int fd = fileno(stream);
  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0) {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After fread...");

    //   errno = EISDIR;
    //   return -1;
    // }

    int buf_size = size * nmemb;

    debug_info("[BYPASS]\t try to xpn_read "<<virtual_fd.real_fd<<", "<<ptr<<", "<<buf_size);

    ret = xpn_read(virtual_fd.real_fd, ptr, buf_size);
    ret = ret / size; // Number of items read

    debug_info("[BYPASS]\t xpn_read "<<virtual_fd.real_fd<<", "<<ptr<<", "<<buf_size<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(fread) "<<ptr<<", "<<size<<" ,"<<nmemb<<" ,"<<stream);

    ret = PROXY(fread)(ptr, size, nmemb, stream);

    debug_info("[BYPASS]\t PROXY(fread) "<<ptr<<", "<<size<<" ,"<<nmemb<<" ,"<<stream<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After fread...");

  return ret;
}

size_t fwrite ( const void *ptr, size_t size, size_t nmemb, FILE *stream )
{         
  size_t ret = (size_t) -1;

  debug_info("[BYPASS] >> Begin fwrite...");
  debug_info("[BYPASS]    1) ptr="<<ptr);
  debug_info("[BYPASS]    2) size="<<size);
  debug_info("[BYPASS]    3) nmemb="<<nmemb);
  debug_info("[BYPASS]    4) stream="<<stream);

  int fd = fileno(stream);
  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    // if (virtual_fd.is_file == 0) {
    //   debug_error("[BYPASS:"<<<<":%d] Error: is not a file\n", __FILE__, __LINE__);
    //   debug_info("[BYPASS] << After fwrite...");

    //   errno = EISDIR;
    //   return -1;
    // }

    int buf_size = size * nmemb;

    debug_info("[BYPASS]\t try to xpn_write "<<virtual_fd.real_fd<<", "<<ptr<<", "<<buf_size);
    ret = xpn_write(virtual_fd.real_fd, ptr, buf_size);
    ret = ret / size; // Number of items Written

    debug_info("[BYPASS]\t xpn_write "<<virtual_fd.real_fd<<", "<<ptr<<", "<<buf_size<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(fwrite) "<<ptr<<", "<<size<<" ,"<<nmemb<<" ,"<<stream);

    ret = PROXY(fwrite)(ptr, size, nmemb, stream);

    debug_info("[BYPASS]\t PROXY(fwrite) "<<ptr<<", "<<size<<" ,"<<nmemb<<" ,"<<stream<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After fwrite...");

  return ret;
}

int fseek ( FILE *stream, long int offset, int whence )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin fseek...");
  debug_info("[BYPASS]    1) stream="<<stream);
  debug_info("[BYPASS]    2) offset="<<offset);
  debug_info("[BYPASS]    3) whence="<<whence);

  int fd = fileno(stream);
  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd<<", "<<offset<<", "<<whence);

    ret = xpn_lseek(virtual_fd.real_fd, offset, whence);

    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd<<", "<<offset<<", "<<whence<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(fseek) "<<stream<<", "<<offset<<", "<<whence);

    ret = PROXY(fseek)(stream, offset, whence);

    debug_info("[BYPASS]\t PROXY(fseek) "<<stream<<", "<<offset<<", "<<whence<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After fseek...");

  return ret;
}

long ftell ( FILE *stream )
{
  debug_info("[BYPASS] >> Begin ftell....");
  debug_info("[BYPASS]    1) stream = "<<stream);

  long ret = -1;

  int fd = fileno(stream);
  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd);

    ret = xpn_lseek(virtual_fd.real_fd, 0, SEEK_CUR);

    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(ftell)");

    ret = PROXY(ftell)(stream);

    debug_info("[BYPASS]\t PROXY(ftell) -> "<<ret);
  }

  debug_info("[BYPASS] << After ftell....");

  return ret;
}

void rewind (FILE *stream)
{
  debug_info("[BYPASS] >> Begin rewind....");
  debug_info("[BYPASS]    1) stream = "<<stream);

  int fd = fileno(stream);
  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd);

    xpn_lseek(virtual_fd.real_fd, 0, SEEK_SET);

    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(rewind)");

    PROXY(rewind)(stream);

    debug_info("[BYPASS]\t PROXY(rewind)");
  }

  debug_info("[BYPASS] << After rewind....");
}

int  feof(FILE *stream)
{
  debug_info("[BYPASS] >> Begin feof....");
  debug_info("[BYPASS]    1) stream = "<<stream);

  int ret = -1;

  int fd = fileno(stream);
  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd);

    int ret1, ret2;
    ret1 = xpn_lseek(virtual_fd.real_fd, 0, SEEK_CUR);
    if (ret != -1) {
      debug_info("[BYPASS] << After feof....");

      return ret;
    }
    ret2 = xpn_lseek(virtual_fd.real_fd, 0, SEEK_END);
    if (ret != -1) {
      debug_info("[BYPASS] << After feof....");

      return ret;
    }

    if (ret1 != ret2) {
      ret = 0;
    }
    else {
      ret = 1;
    }

    debug_info("[BYPASS]\t xpn_lseek "<<virtual_fd.real_fd<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(feof)");

    ret = PROXY(feof)(stream);

    debug_info("[BYPASS]\t PROXY(feof) -> "<<ret);
  }

  debug_info("[BYPASS] << After feof....");

  return ret;
}

// Directory API

int mkdir ( const char *path, mode_t mode )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin mkdir...");
  debug_info("[BYPASS]    1) Path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_mkdir "<<(skip_xpn_prefix(path)));

    ret = xpn_mkdir( (skip_xpn_prefix(path)) ,mode );

    debug_info("[BYPASS]\t xpn_mkdir "<<skip_xpn_prefix(path)<<" -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(mkdir)");

    ret = PROXY(mkdir)((char *)path,mode);

    debug_info("[BYPASS]\t PROXY(mkdir) -> "<<ret);
  }

  debug_info("[BYPASS] << After mkdir...");

  return ret;
}

DIR *opendir ( const char *dirname )
{
  DIR * ret;

  debug_info("[BYPASS] >> Begin opendir...");
  debug_info("[BYPASS]    1) dirname "<<dirname);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if(is_xpn_prefix(dirname))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_opendir");

    ret = xpn_opendir((const char *)(dirname+xpn_prefix_strlen));
    if (ret != NULL) {
      fdsdirtable_put ( ret );
    }

    debug_info("[BYPASS]\t xpn_opendir -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try PROXY(opendir)");
    ret = PROXY(opendir)((char *)dirname);
    debug_info("[BYPASS]\t PROXY(opendir) -> "<<ret);
  }

  debug_info("[BYPASS] << After opendir...");
  return ret;
}

struct dirent *readdir ( DIR *dirp )
{
  struct dirent *ret;

  debug_info("[BYPASS] >> Begin readdir...");
  debug_info("[BYPASS]    1) dirp "<<dirp);

  if (fdsdirtable_get( dirp ) != -1)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_readdir");

    DIR aux_dirp = fdsdirtable_getfd( dirp );
    ret = xpn_readdir(&aux_dirp);

    debug_info("[BYPASS]\t xpn_readdir -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(readdir)");

    ret = PROXY(readdir)(dirp);

    debug_info("[BYPASS]\t PROXY(readdir) -> "<<ret);
  }

  debug_info("[BYPASS] << After readdir...");

  return ret;
}

struct dirent64 *readdir64 ( DIR *dirp )
{
  struct dirent *aux;
  struct dirent64 *ret = NULL;

  debug_info("[BYPASS] >> Begin readdir64...");
  debug_info("[BYPASS]    1) dirp "<<dirp);

  if (fdsdirtable_get( dirp ) != -1)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_readdir");

    DIR aux_dirp = fdsdirtable_getfd( dirp );
    aux = xpn_readdir(&aux_dirp);
    if (aux != NULL)
    {
      ret = (struct dirent64 *)malloc(sizeof(struct dirent64)); // TODO: change to static memory per dir... or where memory is free?
      ret->d_ino    = (__ino64_t)  aux->d_ino;
      ret->d_off    = (__off64_t)  aux->d_off;
      ret->d_reclen =              aux->d_reclen;
      ret->d_type   =              aux->d_type;
      strcpy(ret->d_name, aux->d_name);
    }

    debug_info("[BYPASS]\t xpn_readdir -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(readdir64)");

    ret = PROXY(readdir64)(dirp);

    debug_info("[BYPASS]\t PROXY(readdir64) -> "<<ret);
  } 

  debug_info("[BYPASS] << After readdir64...");

  return ret;
}

int closedir ( DIR *dirp )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin closedir...");
  debug_info("[BYPASS]    1) dirp "<<dirp);

  if( fdsdirtable_get( dirp ) != -1 )
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_closedir");

    fdsdirtable_remove( dirp );
    ret = xpn_closedir( dirp );

    debug_info("[BYPASS]\t xpn_closedir -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(closedir)");

    ret = PROXY(closedir)(dirp);

    debug_info("[BYPASS]\t PROXY(closedir) -> "<<ret);
  }

  debug_info("[BYPASS] << After closedir...");

  return ret;
}

int rmdir ( const char *path )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin rmdir...");
  debug_info("[BYPASS]    1) path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS]\t xpn_rmdir");

    ret = xpn_rmdir( (skip_xpn_prefix(path)) );

    debug_info("[BYPASS]\t xpn_rmdir -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(rmdir)");

    ret = PROXY(rmdir)((char *)path);

    debug_info("[BYPASS]\t PROXY(rmdir) -> "<<ret);
  }

  debug_info("[BYPASS] << After rmdir...");

  return ret;
}

// Proccess API

pid_t fork ( void )
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin fork()");
  debug_info("[BYPASS]\t try to PROXY(fork)");

  ret = PROXY(fork)();
  if(0 == ret) {
    // We want the children to be initialized
    xpn_adaptor_initCalled = 0;
  }

  debug_info("[BYPASS]\t PROXY(fork) -> "<<ret);
  debug_info("[BYPASS] << After fork()");

  return ret;
}

int pipe ( int pipefd[2] )
{
  debug_info("[BYPASS] >> Begin pipe()");
  debug_info("[BYPASS]    1) fd1 "<<pipefd[0]);
  debug_info("[BYPASS]    2) fd2 "<<pipefd[1]);
  debug_info("[BYPASS]\t try to PROXY(pipe)");

  int ret = PROXY(pipe)(pipefd);

  debug_info("[BYPASS]\t PROXY(pipe) -> "<<ret);
  debug_info("[BYPASS] << After pipe()");

  return ret;
}

int dup ( int fd )
{
  debug_info("[BYPASS] >> Begin dup...");
  debug_info("[BYPASS]    1) fd "<<fd);

  int ret = -1;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_dup");

    ret = xpn_dup(virtual_fd.real_fd);

    debug_info("[BYPASS]\t xpn_dup -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] PROXY(dup)");

    ret = PROXY(dup)(fd);

    debug_info("[BYPASS]\t PROXY(dup) -> "<<ret);
  }

  debug_info("[BYPASS] << After dup()");

  return ret;
}

int dup2 ( int fd, int fd2 )
{
  debug_info("[BYPASS] >> Begin dup2...");
  debug_info("[BYPASS]    1) fd "<<fd);
  debug_info("[BYPASS]    2) fd2 "<<fd2);

  int ret = -1;

  struct generic_fd virtual_fd  = fdstable_get ( fd );
  struct generic_fd virtual_fd2 = fdstable_get ( fd2 );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_dup2");

    ret = xpn_dup2(virtual_fd.real_fd, virtual_fd2.real_fd);

    debug_info("[BYPASS]\t xpn_dup2 -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] PROXY(dup2)");

    ret = PROXY(dup2)(fd, fd2);

    debug_info("[BYPASS]\t PROXY(dup2) -> "<<ret);
  }

  debug_info("[BYPASS] << After dup2()");

  return ret;
}

void exit ( int status )
{
  debug_info("[BYPASS] >> Begin exit...");
  debug_info("[BYPASS]    1) status "<<status);

  if (xpn_adaptor_initCalled == 1)
  {
    debug_info("[BYPASS] xpn_destroy");

    xpn_destroy();
  }

  debug_info("[BYPASS] PROXY(exit)");

  PROXY(exit)(status);
  __builtin_unreachable();

  debug_info("[BYPASS] << After exit()");
}

// Manager API

int chdir ( const char *path )
{
  debug_info("[BYPASS] >> Begin chdir...");
  debug_info("[BYPASS]    1) path "<<path);

  int ret = -1;

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_chdir");

    ret = xpn_chdir((char *)skip_xpn_prefix(path));

    debug_info("[BYPASS]\t xpn_chdir -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else 
  {
    debug_info("[BYPASS] PROXY(chdir)");

    ret = PROXY(chdir)((char *)path);

    debug_info("[BYPASS]\t PROXY(chdir) -> "<<ret);
  }

  debug_info("[BYPASS] << After chdir()");

  return ret;
}

int chmod ( const char *path, mode_t mode )
{
  debug_info("[BYPASS] >> Begin chmod...");
  debug_info("[BYPASS]    1) path "<<path);

  int ret = -1;

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_chmod");

    ret = xpn_chmod(skip_xpn_prefix(path), mode);

    debug_info("[BYPASS]\t xpn_chmod -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] PROXY(chmod)");

    ret = PROXY(chmod)((char *)path, mode);

    debug_info("[BYPASS]\t PROXY(chmod) -> "<<ret);
  }

  debug_info("[BYPASS] << After chmod()");

  return ret;
}

int fchmod ( int fd, mode_t mode )
{
  debug_info("[BYPASS] >> Begin fchmod...");
  debug_info("[BYPASS]    1) fd "<<fd);

  int ret = -1;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_fchmod");

    ret = xpn_fchmod(fd,mode);

    debug_info("[BYPASS]\t PROXY(fchmod) -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] PROXY(fchmod)");

    ret = PROXY(fchmod)(fd,mode);

    debug_info("[BYPASS]\t PROXY(fchmod) -> "<<ret);
  }

  debug_info("[BYPASS] << After fchmod()");

  return ret;
}

int chown ( const char *path, uid_t owner, gid_t group )
{
  debug_info("[BYPASS] >> Begin chown...");
  debug_info("[BYPASS]    1) path "<<path);

  int ret = -1;

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_chown");
    ret = xpn_chown(skip_xpn_prefix(path), owner, group);
    debug_info("[BYPASS]\t xpn_chown -> "<<ret);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] PROXY(chown)");
    ret = PROXY(chown)((char *)path, owner, group);
    debug_info("[BYPASS]\t PROXY(chown) -> "<<ret);
  }

  debug_info("[BYPASS] << After chown()");

  return ret;
}

int fcntl ( int fd, int cmd, long arg ) //TODO
{
  debug_info("[BYPASS] >> Begin fcntl...");
  debug_info("[BYPASS]    1) fd "<<fd);
  debug_info("[BYPASS]    2) cmd "<<cmd);
  debug_info("[BYPASS]    3) arg "<<arg);

  int ret = -1;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    debug_info("[BYPASS] xpn_fcntl");

    //TODO
    ret = 0;

    debug_info("[BYPASS]\t xpn_fcntl -> "<<ret);
  }
  else
  {
    debug_info("[BYPASS] PROXY(fcntl)");

    ret = PROXY(fcntl)(fd, cmd, arg);

    debug_info("[BYPASS]\t PROXY(fcntl) -> "<<ret);
  }

  debug_info("[BYPASS] << After fcntl()");

  return ret;
}

int access ( const char *path, int mode )
{
  struct stat64 stats;

  debug_info("[BYPASS] >> Begin access...");
  debug_info("[BYPASS]    1) path "<<path);
  debug_info("[BYPASS]    2) mode "<<mode);

  int ret = -1;

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_access");

    if (__lxstat64(_STAT_VER, path, &stats)) {
      debug_info("[BYPASS]\t xpn_access -> -1");
      debug_info("[BYPASS] << After access()");

      return -1;
    }

    if (mode == F_OK) {
      debug_info("[BYPASS]\t xpn_access -> 0");
      debug_info("[BYPASS] << After access()");

      return 0;
    }

    if ((mode & X_OK) == 0 || (stats.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
    {
      debug_info("[BYPASS]\t xpn_access -> 0");
      debug_info("[BYPASS] << After access()");

      return 0;
    }
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] PROXY(access)");

    ret = PROXY(access)(path, mode);

    debug_info("[BYPASS]\t PROXY(access) -> "<<ret);
  }

  debug_info("[BYPASS] << After access()");

  return ret;
}

char *realpath ( const char *__restrict__ path, char *__restrict__ resolved_path )
{
  debug_info("[BYPASS] >> Begin realpath...");
  debug_info("[BYPASS] 1) Path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_realpath...");

    strcpy(resolved_path, path);

    return resolved_path;
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] Begin PROXY(realpath)...");

    return PROXY(realpath)(path, resolved_path);
  }
}

char * __realpath_chk ( const char * path, char * resolved_path, __attribute__((__unused__)) size_t resolved_len )
{
  debug_info("[BYPASS] >> Begin __realpath_chk...");
  debug_info("[BYPASS] 1) Path "<<path);

  // TODO: taken from https://refspecs.linuxbase.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/libc---realpath-chk-1.html
  // -> ... If resolved_len is less than PATH_MAX, then the function shall abort, and the program calling it shall exit.
  //
  //if (resolved_len < PATH_MAX) {
  //    return -1;
  //}

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn___realpath_chk...");

    strcpy(resolved_path, path);

    return resolved_path;
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] Begin PROXY(realpath)...");

    return PROXY(realpath)(path, resolved_path);
  }
}

int fsync ( int fd ) //TODO
{ 
  debug_info("[BYPASS] >> Begin fsync...");
  debug_info("[BYPASS] 1) fd "<<fd);

  int ret = -1;

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    debug_info("[BYPASS] xpn_fsync");

    //TODO
    ret = 0;

    debug_info("[BYPASS]\t xpn_fsync -> "<<ret);
  }
  else
  {
    debug_info("[BYPASS] PROXY(fsync)");

    ret = PROXY(fsync)(fd);

    debug_info("[BYPASS]\t PROXY(fsync) -> "<<ret);
  }

  debug_info("[BYPASS] << After fsync()");

  return ret;
}

int flock(int fd, int operation)
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin flock...");
  debug_info("[BYPASS]    * fd="<<fd);
  debug_info("[BYPASS]    * operation="<<operation);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    //TODO
    return 0;
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(flock) "<<fd<<", "<<operation);

    ret = PROXY(flock)(fd, operation);

    debug_info("[BYPASS]\t PROXY(flock) "<<fd<<", "<<operation<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After flock...");
  
  return ret;
}

int statvfs(const char * path, struct statvfs * buf)
{
  debug_info("[BYPASS] >> Begin statvfs...");
  debug_info("[BYPASS] 1) Path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_statvfs...");

    return xpn_statvfs(skip_xpn_prefix(path), buf);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] Begin PROXY(statvfs)...");

    return PROXY(statvfs)(path, buf);
  }
}

int fstatvfs(int fd, struct statvfs * buf)
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin fstatvfs...");
  debug_info("[BYPASS]    * fd="<<fd);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    ret = xpn_fstatvfs(fd, buf);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(fstatvfs) "<<fd);

    ret = PROXY(fstatvfs)(fd, buf);

    debug_info("[BYPASS]\t PROXY(fstatvfs) "<<fd<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After fstatvfs...");
  
  return ret;
}

int statfs(const char * path, struct statfs * buf)
{
  debug_info("[BYPASS] >> Begin statfs...");
  debug_info("[BYPASS] 1) Path "<<path);

  // This if checks if variable path passed as argument starts with the expand prefix.
  if (is_xpn_prefix(path))
  {
    // We must initialize expand if it has not been initialized yet.
    xpn_adaptor_keepInit ();

    // It is an XPN partition, so we redirect the syscall to expand syscall
    debug_info("[BYPASS] xpn_statfs...");

    return xpn_statfs(skip_xpn_prefix(path), buf);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS] Begin PROXY(statfs)...");

    return PROXY(statfs)(path, buf);
  }
}

int fstatfs(int fd, struct statfs * buf)
{
  int ret = -1;

  debug_info("[BYPASS] >> Begin fstatfs...");
  debug_info("[BYPASS]    * fd="<<fd);

  struct generic_fd virtual_fd = fdstable_get ( fd );

  // This if checks if variable fd passed as argument is a expand fd.
  if(virtual_fd.type == FD_XPN)
  {
    ret = xpn_fstatfs(fd, buf);
  }
  // Not an XPN partition. We must link with the standard library
  else
  {
    debug_info("[BYPASS]\t try to PROXY(fstatfs) "<<fd);

    ret = PROXY(fstatfs)(fd, buf);

    debug_info("[BYPASS]\t PROXY(fstatfs) "<<fd<<" -> "<<ret);
  }

  debug_info("[BYPASS] << After fstatfs...");
  
  return ret;
}
// MPI API

int MPI_Init ( int *argc, char ***argv )
{
  char *value;

  debug_info("[BYPASS] >> Begin MPI_Init");

  // We must initialize expand if it has not been initialized yet.
  xpn_adaptor_keepInit ();

  // It is an XPN partition, so we redirect the syscall to expand syscall
  value = getenv("XPN_IS_MPI_SERVER");
  if (NULL == value) {
    debug_info("[BYPASS] << After MPI_Init");

    return PMPI_Init(argc, argv);
  }

  debug_info("[BYPASS] << After MPI_Init");

  return MPI_SUCCESS;
}

int MPI_Init_thread ( int *argc, char ***argv, int required, int *provided )
{
  char *value;

  debug_info("[BYPASS] >> Begin MPI_Init_thread");

  // We must initialize expand if it has not been initialized yet.
  xpn_adaptor_keepInit ();

  // It is an XPN partition, so we redirect the syscall to expand syscall
  value = getenv("XPN_IS_MPI_SERVER");
  if (NULL == value) {
    debug_info("[BYPASS] << After MPI_Init_thread");

    return PMPI_Init_thread( argc, argv, required, provided );
  }
  debug_info("[BYPASS] << After MPI_Init_thread");

  return MPI_SUCCESS;
}

int MPI_Finalize (void)
{
  char *value;

  debug_info("[BYPASS] >> Begin MPI_Finalize");

  value = getenv("XPN_IS_MPI_SERVER");
  if (NULL != value && xpn_adaptor_initCalled == 1) {
    debug_info("[BYPASS] xpn_destroy");

    xpn_destroy();
  }

  debug_info("[BYPASS] << After MPI_Finalize");

  return PMPI_Finalize();
}

// Used with domainname as the command, msgid as the host_list and category as the rank
char * dcgettext (const char *domainname, const char *msgid, int category)
{
  debug_info("[BYPASS] >> Begin dgettext...");
  debug_info("[BYPASS] 1) "<<domainname);
  debug_info("[BYPASS] 2) "<<msgid);
  debug_info("[BYPASS] 3) "<<category);

  if (strcmp(domainname, "start expand") == 0)
  {
    xpn_start_expand(msgid, category);
    return NULL;
  }
  else if (strcmp(domainname, "end expand") == 0)
  {
    xpn_end_expand(msgid, category);
    return NULL;
  }
  else if (strcmp(domainname, "start shrink") == 0)
  {
    xpn_start_shrink(msgid, category);
    return NULL;
  }
  else if (strcmp(domainname, "end shrink") == 0)
  {
    xpn_end_shrink(msgid, category);
    return NULL;
  }
  else
  {
    fprintf(stderr, "Error: todo bypass dgettext");
    raise(SIGTERM);
    return NULL;
  }
}

#ifdef __cplusplus
}
#endif