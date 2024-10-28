
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
#include "base_c/filesystem.h"
#include <csignal>

namespace XPN
{
fabric_server_control_comm::fabric_server_control_comm ()
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_control_comm] >> Begin");
  
  fabric::init(m_domain);

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_control_comm] >> End");
}

fabric_server_control_comm::~fabric_server_control_comm()
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [~fabric_server_control_comm] >> Begin");

  fabric::destroy(m_domain);

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [~fabric_server_control_comm] >> End");
}

xpn_server_comm* fabric_server_control_comm::accept ( int socket )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] >> Begin");

  // // Accept
  // debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_accept] Accept");

  // sc = ::accept(m_socket, (struct sockaddr *)&client_addr, &size);
  // if (sc < 0)
  // {
  //   print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_destroy] ERROR: accept fails");
  //   return nullptr;
  // }

  // debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_destroy] desp. accept conection from "<<sc);
  // // tcp_nodelay
  // flag = 1;
  // ret = ::setsockopt(sc, IPPROTO_TCP, TCP_NODELAY, & flag, sizeof(flag));
  // if (ret < 0)
  // {
  //   perror("setsockopt: ");
  //   return nullptr;
  // }

  // //NEW
  // int val = MAX_BUFFER_SIZE; //1 MB
  // ret = ::setsockopt(sc, SOL_SOCKET, SO_SNDBUF, (char *)&val, sizeof(int));
  // if (ret < 0)
  // {
  //   perror("setsockopt: ");
  //   return nullptr;
  // }

  // val = MAX_BUFFER_SIZE; //1 MB
  // ret = ::setsockopt(sc, SOL_SOCKET, SO_RCVBUF, (char *)&val, sizeof(int));
  // if (ret < 0)
  // {
  //   perror("setsockopt: ");
  //   return nullptr;
  // }


  int ret = 0;
  fabric::comm new_comm;
  
  ret = socket::send(socket, m_port_name.data(), MAX_PORT_NAME);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: socket send port fails");
    return nullptr;
  }

  ret = fabric::new_comm(m_domain, new_comm);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: fabric new_comm fails");
    return nullptr;
  }

  // First send the server address
  size_t ad_len = MAX_PORT_NAME;
  char ad_buff[MAX_PORT_NAME];
  ret = fabric::get_addr(new_comm, ad_buff, ad_len);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: fabric get_addr fails");
    return nullptr;
  }

  ret = socket::send(socket, &ad_len, sizeof(ad_len));
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: socket send addres size fails");
    return nullptr;
  }
  
  ret = socket::send(socket, ad_buff, ad_len);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: socket send addres fails");
    return nullptr;
  }
  
  // Second recv the client address
  ret = socket::recv(socket, &ad_len, sizeof(ad_len));
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: socket recv addres size fails");
    return nullptr;
  }
  
  ret = socket::recv(socket, ad_buff, ad_len);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: socket recv addres fails");
    return nullptr;
  }

  ret = fabric::register_addr(new_comm, ad_buff);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] ERROR: fabric register_addr fails");
    return nullptr;
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_CONTROL_COMM] [fabric_server_control_comm_accept] << End");

  return new (std::nothrow) fabric_server_comm(new_comm);
}


void fabric_server_control_comm::disconnect ( xpn_server_comm* comm )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] >> Begin");
  
  fabric_server_comm *in_comm = static_cast<fabric_server_comm*>(comm);

  fabric::close(in_comm->m_comm);

  delete comm;

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_disconnect] << End");
}


int64_t fabric_server_comm::read_operation ( xpn_server_ops &op, int &rank_client_id, int &tag_client_id )
{
  int ret;
  int msg[2];

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] >> Begin");

  // Get message
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] Read operation");
  ret = fabric::recv(m_comm, msg, sizeof(msg));
  if (ret < 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] ERROR: read fails");
  }

  rank_client_id = 0;
  tag_client_id  = msg[0];
  op             = static_cast<xpn_server_ops>(msg[1]);

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] read (SOURCE "<<m_socket<<", MPI_TAG "<<tag_client_id<<", MPI_ERROR "<<ret<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_operation] << End");

  // Return OK
  return 0;
}

int64_t fabric_server_comm::read_data ( void *data, int64_t size, [[maybe_unused]] int rank_client_id, [[maybe_unused]] int tag_client_id )
{
  int ret;

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

  ret = fabric::recv(m_comm, data, size);
  if (ret < 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] ERROR: read fails");
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] read (SOURCE "<<m_socket<<", MPI_TAG "<<tag_client_id<<", MPI_ERROR "<<ret<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_read_data] << End");

  // Return bytes read
  return size;
}

int64_t fabric_server_comm::write_data ( const void *data, int64_t size, [[maybe_unused]] int rank_client_id, [[maybe_unused]] int tag_client_id )
{
  int ret;

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

  ret = fabric::send(m_comm, data, size);
  if (ret < 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] ERROR: MPI_Send fails");
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [FABRIC_SERVER_COMM] [fabric_server_comm_write_data] << End");

  // Return bytes written
  return size;
}

} // namespace XPN