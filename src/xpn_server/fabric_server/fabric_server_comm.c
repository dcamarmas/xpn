
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

#include "fabric_server_comm.h"


/* ... Const / Const ................................................. */


/* ... Global variables / Variables globales ........................ */


/* ... Functions / Funciones ......................................... */

// init, destroy

// accept, disconnect
int fabric_server_comm_accept ( struct fabric_domain *fabric_domain, char *dest_addr, char *port_name, struct fabric_comm **new_sd )
{
  int ret;

  *new_sd = malloc(sizeof(struct fabric_comm));
  if (*new_sd == NULL) {
    printf("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] ERROR: Memory allocation\n", 0);
    return -1;
  }

  debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] >> Begin\n", 0);

  // Accept
  debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] Accept %s %s\n", 0, dest_addr, port_name);

  ret = fabric_new_comm(fabric_domain, *new_sd);
  if (ret < 0)
  {
    printf("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] ERROR: fabric_init fails\n", 0);
    return -1;
  }

  debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] << End %ld\n", 0, **new_sd);

  return 0;
}

int fabric_server_comm_disconnect ( struct fabric_comm *fd )
{
  int ret;

  debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] >> Begin\n", 0);

  if (fd == NULL)
  {
    printf("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] ERROR: The fabric_comm is NULL\n", 0);
    return -1;
  }

  // Disconnect
  debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] Disconnect\n", 0);

  ret = fabric_close_comm(fd);
  if (ret < 0)
  {
    printf("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] ERROR: MPI_Comm_disconnect fails\n", 0);
    return -1;
  }
  free(fd);
  debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] << End\n", 0);

  // Return OK
  return 0;
}

// ssize_t fabric_server_comm_read_operation ( struct fabric_comm *fd, int *op, int *rank_client_id, int *tag_client_id )
// {
//   int ret;
//   int msg[2];

//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] >> Begin\n", 0);

//   // Get message
//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] Read operation %p\n", 0, fd);

//   ret = fabric_recv(fd, msg, sizeof(msg));
//   if (ret < 0) {
//     debug_warning("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] ERROR: fabric_recv fails\n", 0);
//   }

//   *rank_client_id = 0;
//   *tag_client_id  = msg[0];
//   *op             = msg[1];

//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] fabric_recv (SOURCE %d, TAG %d, OP %d, ERROR %d)\n", 0, *rank_client_id, *rank_client_id, *op, ret);
//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] << End\n", 0);

//   // Return OK
//   return 0;
// }


// ssize_t fabric_server_comm_write_data ( struct fabric_comm *fd, char *data, ssize_t size, int rank_client_id, int tag_client_id )
// {
//   int ret;

//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] >> Begin\n", 0);

//   if (size == 0) {
//       return 0;
//   }
//   if (size < 0)
//   {
//     printf("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] ERROR: size < 0\n", 0);
//     return -1;
//   }

//   // Send message
//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] Write data tag %d\n", 0, tag_client_id);

//   ret = fabric_send(fd, data, size);
//   if (ret < 0) {
//     debug_warning("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] ERROR: fabric_send fails\n", 0);
//   }

//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] << End\n", 0);

//   // Return bytes written
//   return size;
// }

// ssize_t fabric_server_comm_read_data ( struct fabric_comm *fd, char *data, ssize_t size, int rank_client_id, int tag_client_id )
// {
//   int ret;

//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] >> Begin\n", 0);

//   if (size == 0) {
//     return  0;
//   }
//   if (size < 0)
//   {
//     printf("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] ERROR: size < 0\n", 0);
//     return  -1;
//   }

//   // Get message
//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] Read data tag %d\n", 0, tag_client_id);

//   ret = fabric_recv(fd, data, size);
//   if (ret < 0) {
//     debug_warning("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] ERROR: fabric_recv fails\n", 0);
//   }

//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] fabric_recv (ERROR %d)\n", 0, ret);
//   debug_info("[Server=%d] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] << End\n", 0);

//   // Return bytes read
//   return size;
// }


/* ................................................................... */

