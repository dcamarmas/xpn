
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

#ifndef _XPN_POLICY_OPEN_H
#define _XPN_POLICY_OPEN_H

  #ifdef  __cplusplus
    extern "C" {
  #endif


  /* ... Include / Inclusion ........................................... */

  #include "xpn_file.h"
  #include "xpn_policy_rw.h"


  /* ... Const / Const ................................................. */

  #define XPN_META_SIZE (4*KB)


  /* ... Functions / Funciones ......................................... */

  void XpnGetURLServer( struct nfi_server *serv, const char *abs_path, char *url_serv);

  int XpnGetServers(int pd, int fd, struct nfi_server **servers);

  int XpnGetFh(struct xpn_metadata *mdata, struct nfi_fhandle **fh,  struct nfi_server *servers,  char *path);
  int XpnGetFhDir(struct xpn_metadata *mdata, struct nfi_fhandle **fh,  struct nfi_server *servers,  char *path);

  int XpnGetAtribFd   (int fd,      struct stat *st);
  int XpnGetAtribPath (char * path, struct stat *st);


  /* ................................................................... */

  #ifdef  __cplusplus
    }
  #endif

#endif
