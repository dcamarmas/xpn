
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


#ifndef _XPN_UTILS_H
#define _XPN_UTILS_H

  #ifdef  __cplusplus
    extern "C" {
  #endif


  /* ... Include / Inclusion ........................................... */

     #include "xpn.h"
     #include "xpn_file.h"
     #include "xpn_policy_open.h"
     #include "xpn_policy_cwd.h"
     #include "xpn_init.h"
     #include "xpn_rw.h"
     #include "base/workers.h"


  /* ... Functions / Funciones ......................................... */

     int   xpn_simple_preload ( const char *virtual_path, const char *storage_path ) ;
     int   xpn_simple_flush   ( const char *virtual_path, const char *storage_path ) ;


  /* ................................................................... */

  #ifdef  __cplusplus
    }
  #endif

#endif
