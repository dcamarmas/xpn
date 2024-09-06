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

#include "nfi_fabric_server_comm.h"
#include "socket.h"
#include "fabric.h"
#include "xpn_server/xpn_server_conf.h"


/* ... Const / Const ................................................. */


/* ... Global variables / Variables globales ........................ */


/* ... Functions / Funciones ......................................... */

int nfi_fabric_server_comm_connect ( struct fabric_domain *fabric_domain, char * srv_name, char * port_name, struct fabric_comm *out_fabric_comm )
{
  int ret;
  int connection_socket;
  char addr_buf[64];
  size_t addr_buf_size = 64, addr_buf_size_aux = 64;

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] >> Begin\n");
  
  ret = fabric_new_comm(fabric_domain, out_fabric_comm);
  if (ret < 0){
    printf("Error: fabric_new_comm %d\n", ret);
    return ret;
  }

  // Lookup port name
  ret = socket_client_connect(srv_name, &connection_socket);
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: socket connect\n");
    return -1;
  }
  int buffer = SOCKET_ACCEPT_CODE;
  ret = socket_send(connection_socket, &buffer, sizeof(buffer));
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: socket send\n");
    socket_close(connection_socket);
    return -1;
  }
  ret = socket_recv(connection_socket, port_name, XPN_SERVER_MAX_PORT_NAME);
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: socket read\n");
    socket_close(connection_socket);
    return -1;
  }

  ret = socket_recv(connection_socket, addr_buf, addr_buf_size);
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: socket read\n");
    socket_close(connection_socket);
    return -1;
  }

  ret = fabric_register_addr(out_fabric_comm, addr_buf);

  ret = fabric_get_addr(out_fabric_comm, addr_buf, addr_buf_size_aux);
  if (ret < 0){
    printf("Error: fabric_get_addr\n");
    return ret;
  }
  
  ret = socket_send(connection_socket, addr_buf, addr_buf_size);
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: socket send\n");
    socket_close(connection_socket);
    return -1;
  }

  socket_close(connection_socket);

  if (ret < 0) {
    printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: Lookup %s Port %s\n", srv_name, port_name);
    return -1;
  }

  debug_info("[NFI_FABRIC_SERVER_COMM] ----SERVER = %s PORT = %s\n", srv_name, port_name);


  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] << End\n");

  return ret;
}

int nfi_fabric_server_comm_disconnect(struct fabric_comm *fabric_comm) 
{
  int ret;
  int code = XPN_SERVER_DISCONNECT;

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] >> Begin\n");

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] Send disconnect message\n");
  ret = fabric_send(fabric_comm, &code, sizeof(code));
  if (ret < 0) {
    printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] ERROR: nfi_fabric_server_comm_write_operation fails\n");
    return ret;
  }

  // Disconnect
  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] Disconnect\n");

  ret = fabric_close_comm(fabric_comm);
  if (ret < 0) {
    printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] ERROR: MPI_Comm_disconnect fails\n");
    return ret;
  }

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] << End\n");

  // Return OK
  return ret;
}
/* ................................................................... */
