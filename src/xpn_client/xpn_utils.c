
/*
 *  Copyright 2000-2026 The Expand Team.
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

     #include "xpn.h"
     #include "xpn_api_mutex.h"
     #include "xpn_utils.h"


  /* ... Functions / Funciones ......................................... */

     //
     // preload - flush
     //

     int xpn_preload(const char *virtual_path, const char *storage_path)
     {
       int ret;

       debug_info("[XPN_STDIO] [xpn_preload] >> Begin\n");

       XPN_API_LOCK();
       ret = xpn_simple_preload(virtual_path, storage_path);
       XPN_API_UNLOCK();

       debug_info("[XPN_STDIO] [xpn_preload] >> End\n");

       return ret;
     }

     int xpn_flush(const char *virtual_path, const char *storage_path)
     {
       int ret;

       debug_info("[XPN_STDIO] [xpn_simple_flush] >> Begin\n");

       XPN_API_LOCK();
       ret = xpn_simple_flush(virtual_path, storage_path);
       XPN_API_UNLOCK();

       debug_info("[XPN_STDIO] [xpn_simple_flush] >> End\n");

       return ret;
     }


/* ................................................................... */

