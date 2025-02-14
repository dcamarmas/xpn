
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

#include "fabric_server_comm.hpp"
#include "base_cpp/debug.hpp"
#include "base_cpp/timer.hpp"
#include "base_cpp/ns.hpp"
#include "base_cpp/socket.hpp"
#include <csignal>
#include "lfi.h"
#include "lfi_async.h"
#include <cassert>

namespace XPN
{
fabric_server_control_comm::fabric_server_control_comm ()
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_control_comm] >> Begin");
  
  int port = 0;
  m_server_comm = lfi_server_create(NULL, &port);

  m_port_name = std::to_string(port);

  if (m_server_comm < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_init] ERROR: bind fails");
    std::raise(SIGTERM);
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_control_comm] >> End");
}

fabric_server_control_comm::~fabric_server_control_comm()
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [~fabric_server_control_comm] >> Begin");

  lfi_server_close(m_server_comm);

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [~fabric_server_control_comm] >> End");
}

xpn_server_comm* fabric_server_control_comm::accept ( int socket )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] >> Begin");

  int ret = 0;
  
  ret = socket::send(socket, m_port_name.data(), MAX_PORT_NAME);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: socket send port fails");
    return nullptr;
  }

  int new_comm = lfi_server_accept(m_server_comm);
  if (new_comm < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: accept fails");
    return nullptr;
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] << End");

  return new (std::nothrow) fabric_server_comm(new_comm);
}


void fabric_server_control_comm::disconnect ( xpn_server_comm* comm )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] >> Begin");
  
  fabric_server_comm *in_comm = static_cast<fabric_server_comm*>(comm);

  lfi_client_close(in_comm->m_comm);

  delete comm;

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] << End");
}

void fabric_server_control_comm::disconnect ( int id )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] >> Begin");
  
  lfi_client_close(id);

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] << End");
}


int64_t fabric_server_comm::read_operation ( xpn_server_ops &op, int &rank_client_id, int &tag_client_id )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] >> Begin");

  if (!shm_request){
    shm_request = {lfi_request_create(LFI_ANY_COMM_SHM), lfi_request_free};
    if (!shm_request){
        print("Error shm_request is null");
    }

    if (lfi_trecv_async(shm_request.get(), shm_msg, sizeof(shm_msg), 0) < 0){
        print("Error in lfi_trecv_async")
        return -1;
    }
  }

  if (!peer_request){
    peer_request = {lfi_request_create(LFI_ANY_COMM_PEER), lfi_request_free};
    if (!peer_request){
        print("Error peer_request is null");
    }

    if (lfi_trecv_async(peer_request.get(), peer_msg, sizeof(peer_msg), 0) < 0){
        print("Error in lfi_trecv_async")
        return -1;
    }
  }

  lfi_request *requests[2] = {shm_request.get(), peer_request.get()};

  int completed = lfi_wait_many(requests, 2, 1);

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] request shm  (RANK "<<lfi_request_source(shm_request.get())<<", TAG "<<shm_msg[0]<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] request peer (RANK "<<lfi_request_source(peer_request.get())<<", TAG "<<peer_msg[0]<<")");
  if (completed == 0){
    rank_client_id = lfi_request_source(shm_request.get());
    tag_client_id  = shm_msg[0];
    op             = static_cast<xpn_server_ops>(shm_msg[1]);
    
    shm_msg[0] = -1;
    shm_msg[1] = -1;

    // One option is to free the request and create another one or reuse the requets for a new recv
    // shm_request.release();
    if (lfi_trecv_async(shm_request.get(), shm_msg, sizeof(shm_msg), 0) < 0){
        print("Error in lfi_trecv_async")
        return -1;
    }
  }else if (completed == 1){
    rank_client_id = lfi_request_source(peer_request.get());
    tag_client_id  = peer_msg[0];
    op             = static_cast<xpn_server_ops>(peer_msg[1]);

    peer_msg[0] = -1;
    peer_msg[1] = -1;

    // One option is to free the request and create another one or reuse the requets for a new recv
    // peer_request.release();
    if (lfi_trecv_async(peer_request.get(), peer_msg, sizeof(peer_msg), 0) < 0){
        print("Error in lfi_trecv_async")
        return -1;
    }
  }else{
    return -1;
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] read (SOURCE "<<m_comm<<", RANK "<<rank_client_id<<", TAG "<<tag_client_id<<", MPI_ERROR "<<0<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] << End");

  // Return OK
  return 0;
}

int64_t fabric_server_comm::read_data ( void *data, int64_t size, int rank_client_id, int tag_client_id )
{
  int64_t ret = 0;

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] >> Begin");

  if (size == 0) {
    return  0;
  }
  if (size < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] ERROR: size < 0");
    return  -1;
  }

  // Get message
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] Read data tag "<< tag_client_id);
  ret = lfi_trecv(rank_client_id, data, size, tag_client_id);
  if (ret < 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] ERROR: read fails");
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] read (SOURCE "<<m_comm<<", MPI_TAG "<<tag_client_id<<", MPI_ERROR "<<ret<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] << End");

  // Return bytes read
  return ret;
}

int64_t fabric_server_comm::write_data ( const void *data, int64_t size, int rank_client_id, int tag_client_id )
{
  int64_t ret = 0;

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] >> Begin");

  if (size == 0) {
      return 0;
  }
  if (size < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] ERROR: size < 0");
    return -1;
  }

  // Send message
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] Write data tag "<< tag_client_id);

  ret = lfi_tsend(rank_client_id, data, size, tag_client_id);
  if (ret < 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] ERROR: MPI_Send fails");
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] << End");

  // Return bytes written
  return ret;
}

} // namespace XPN