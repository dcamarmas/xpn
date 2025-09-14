
/*
 *  Copyright 2020-2025 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
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


#ifndef _XPN_DEBUG_H
#define _XPN_DEBUG_H

  #ifdef  __cplusplus
    extern "C" {
  #endif


  /* ... Include / Inclusion ........................................... */

     #include <stdio.h>
     #include <errno.h>
     #include "profiler.h"


  /* ... Const / Const ................................................. */

     // xpn_debug: set the debug configuration to be used
     // For example, used in Expand at src/xpn_client/xpn/xpn_simple/xpncore/xpn_init.c
     extern int xpn_debug;


  /* ... Macros / Macros ............................................... */

     #define PRINT_TRACE \
       if (xpn_debug) { \
         fprintf (stderr, "[%s:%d]\n", __FILE__, __LINE__); \
       }

     #define XPN_DEBUG_COMMON_HEADER fprintf (stderr, "[%s][%s:%d] ", __func__, __FILE__, __LINE__);

     #define XPN_DEBUG(format, ...) \
       if (xpn_debug) { \
         XPN_DEBUG_COMMON_HEADER \
         fprintf (stderr, format, ## __VA_ARGS__); \
         fprintf (stderr, "\n"); \
       }

     #define XPN_DEBUG_BEGIN_CUSTOM(format, ...) \
       if (xpn_debug) { \
         XPN_DEBUG_COMMON_HEADER \
         fprintf (stderr, "Begin %s", __func__); \
         fprintf (stderr, "("); \
         fprintf (stderr, format, ## __VA_ARGS__); \
         fprintf (stderr, ")"); \
         fprintf (stderr, "\n"); \
       }; XPN_PROFILER_DEFAULT_BEGIN();

     #define XPN_DEBUG_END_CUSTOM(format, ...) \
       if (xpn_debug) { \
         XPN_DEBUG_COMMON_HEADER \
         fprintf (stderr, "End   %s", __func__); \
         fprintf (stderr, "("); \
         fprintf (stderr, format, ## __VA_ARGS__); \
         fprintf (stderr, ")"); \
         fprintf (stderr, "errno=%d %s", errno, strerror(errno)); \
         fprintf (stderr, "\n"); \
       }; XPN_PROFILER_DEFAULT_END_CUSTOM(format, ## __VA_ARGS__);

     #define XPN_DEBUG_BEGIN XPN_DEBUG("Begin %s()", __func__); XPN_PROFILER_DEFAULT_BEGIN();
     #define XPN_DEBUG_END   XPN_DEBUG("End   %s(), errno=%d %s", __func__, errno, strerror(errno)); XPN_PROFILER_DEFAULT_END();

     #define XPN_DEBUG_BEGIN_ARGS1(...) XPN_DEBUG("Begin %s(%s)", __func__, ## __VA_ARGS__); XPN_PROFILER_DEFAULT_BEGIN();
     #define XPN_DEBUG_END_ARGS1(...)   XPN_DEBUG("End   %s(%s), errno=%d %s", __func__, ## __VA_ARGS__, errno, strerror(errno)); XPN_PROFILER_DEFAULT_END_CUSTOM("%s", ## __VA_ARGS__);

     #define XPN_DEBUG_BEGIN_ARGS2(...) XPN_DEBUG("Begin %s(%s, %s)", __func__, ## __VA_ARGS__); XPN_PROFILER_DEFAULT_BEGIN();
     #define XPN_DEBUG_END_ARGS2(...)   XPN_DEBUG("End   %s(%s, %s), errno=%d %s", __func__, ## __VA_ARGS__, errno, strerror(errno)); XPN_PROFILER_DEFAULT_END_CUSTOM("%s, %s", ## __VA_ARGS__);


  /* ... Configuration / Configuración ................................. */

     #if defined(DEBUG)
       // base
       // nfi
       #define DEBUG_NFI
       #define DEBUG_MNT
       #define DEBUG_NFS
       #define DEBUG_NFSW
       #define DEBUG_NFS_ERR
       #define DEBUG_NFS_ERRW
       // xpn
       #define XPN_DEBUG
     #endif


  /* ................................................................... */


  #ifdef  __cplusplus
    }
  #endif

#endif

