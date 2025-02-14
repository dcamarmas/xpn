
/*
 *  Copyright 2000-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
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


#ifndef _FABRIC_H_
#define _FABRIC_H_

  #ifdef  __cplusplus
    extern "C" {
  #endif

  /* ... Include / Inclusion ........................................... */

  #include "all_system.h"
  #include "debug_msg.h"
  #include <rdma/fabric.h>
  #include <rdma/fi_domain.h>
  #include <rdma/fi_endpoint.h>
  #include <rdma/fi_cm.h>


  /* ... Const / Const ................................................. */


  /* ... Data structures / Estructuras de datos ........................ */

  struct fabric_domain
  {
    struct fi_info *hints, *info;
    struct fid_fabric *fabric;
    struct fid_domain *domain;
  };

  struct fabric_comm
  {
    struct fabric_domain * fabric_domain;
    struct fid_ep *ep;
    struct fid_av *av;
    struct fid_cq *cq;
    fi_addr_t fi_addr;
  };

  /* ... Functions / Funciones ......................................... */

  int fabric_init ( struct fabric_domain *fabric );

  int fabric_new_comm ( struct fabric_domain *domain, struct fabric_comm *out_fabric_comm );
  
  int fabric_get_addr( struct fabric_comm *fabric_comm, char * out_addr, size_t size_addr );
  int fabric_register_addr( struct fabric_comm *fabric_comm, char * addr_buf );
  int fabric_send ( struct fabric_comm *fabric, void * buffer, size_t size );
  int fabric_recv ( struct fabric_comm *fabric, void * buffer, size_t size );
  int fabric_close ( struct fabric_comm *fabric );
  int fabric_close_comm ( struct fabric_comm *fabric_comm );
  int fabric_destroy ( struct fabric_domain *domain );

  /* ... Macros / Macros .................................................. */

  #ifdef  __cplusplus
    }
  #endif

#endif
