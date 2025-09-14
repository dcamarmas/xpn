
/*
 *  Copyright 2000-2025 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
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


#ifndef _SOCKET_H_
#define _SOCKET_H_

  #ifdef  __cplusplus
    extern "C" {
  #endif


  /* ... Include / Inclusion ........................................... */

     #include "all_system.h"
     #include "debug_msg.h"
     #include "utils.h"
     #include "socket_ip4.h"
     #include "socket_ip6.h"

     #include <arpa/inet.h>
     #include <netdb.h>
     #include <sys/socket.h>
     #include <netinet/in.h>


  /* ... Const / Const ................................................. */

     // values for ip_version
     #define SCK_IP4 4
     #define SCK_IP6 6


  /* ... Functions / Funciones ......................................... */

     int socket_send ( int socket, void * buffer, int size );
     int socket_recv ( int socket, void * buffer, int size );

     int socket_setopt_data    ( int socket ) ;
     int socket_setopt_service ( int socket ) ;

     int socket_server_create  ( int *out_socket, int port,                  int ip_version );
     int socket_server_accept  ( int socket, int *out_conection_socket,      int ip_version );
     int socket_client_connect              ( char *srv_name, int   port,      int *out_socket, int ip_version );
     int socket_client_connect_with_retries ( char *srv_name, char *port_name, int *out_socket, int n_retries, int ip_version ) ;
     int socket_close ( int socket );

     int socket_gethostname   ( char * srv_name, int socket_mode ) ;
     int socket_gethostbyname ( char * ip, size_t ip_size, char * srv_name, int socket_mode ) ;
     int socket_getsockname   ( char * port_name, int in_socket, int socket_mode ) ;


  /* ................................................................... */


  #ifdef  __cplusplus
    }
  #endif

#endif

