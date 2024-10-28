
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

#include "nfi_fabric_server_comm.hpp"
#include "xpn_server/xpn_server_params.hpp"
#include "base_cpp/debug.hpp"
#include "base_cpp/socket.hpp"
#include "base_cpp/ns.hpp"
#include <csignal>
#include <xpn_server/xpn_server_ops.hpp>

namespace XPN {

nfi_fabric_server_control_comm::nfi_fabric_server_control_comm ()
{
  debug_info("[NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm] >> Begin");
  
  fabric::init(m_domain);

  debug_info("[NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm] >> End");
}

nfi_fabric_server_control_comm::~nfi_fabric_server_control_comm()
{
  debug_info("[NFI_FABRIC_SERVER_CONTROL_COMM] [~nfi_fabric_server_control_comm] >> Begin");

  fabric::destroy(m_domain);

  debug_info("[NFI_FABRIC_SERVER_CONTROL_COMM] [~nfi_fabric_server_control_comm] >> End");
}

nfi_xpn_server_comm* nfi_fabric_server_control_comm::connect ( const std::string &srv_name )
{
  int ret;
  int connection_socket;
  fabric::comm new_fabric_comm;
  
  char port_name[MAX_PORT_NAME];

  debug_info("[NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] >> Begin\n");
  
  ret = fabric::new_comm(m_domain, new_fabric_comm);
  if (ret < 0){
    printf("Error: fabric_new_comm %d\n", ret);
    return nullptr;
  }

  // Lookup port name
  ret = socket::client_connect(srv_name, socket::get_xpn_port() ,connection_socket);
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket connect\n");
    return nullptr;
  }
  int buffer = socket::ACCEPT_CODE;
  ret = socket::send(connection_socket, &buffer, sizeof(buffer));
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket send\n");
    socket::close(connection_socket);
    return nullptr;
  }
  ret = socket::recv(connection_socket, port_name, MAX_PORT_NAME);
  if (ret < 0)
  {
    debug_error("[NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket read\n");
    socket::close(connection_socket);
    return nullptr;
  }

  // First recv the server address
  size_t ad_len = MAX_PORT_NAME;
  char ad_buff[MAX_PORT_NAME];
  ret = socket::recv(connection_socket, &ad_len, sizeof(ad_len));
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket recv addres size fails");
    socket::close(connection_socket);
    return nullptr;
  }
  
  ret = socket::recv(connection_socket, ad_buff, ad_len);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket recv addres fails");
    socket::close(connection_socket);
    return nullptr;
  }

  ret = fabric::register_addr(new_fabric_comm, ad_buff);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: fabric register_addr fails");
    socket::close(connection_socket);
    return nullptr;
  }

  // Second send the client address
  ret = fabric::get_addr(new_fabric_comm, ad_buff, ad_len);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: fabric get_addr fails");
    socket::close(connection_socket);
    return nullptr;
  }

  ret = socket::send(connection_socket, &ad_len, sizeof(ad_len));
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket send addres size fails");
    socket::close(connection_socket);
    return nullptr;
  }
  
  ret = socket::send(connection_socket, ad_buff, ad_len);
  if (ret < 0){
    print("[Server="<<ns::get_host_name()<<"] [NFI_FABRIC_SERVER_CONTROL_COMM] [nfi_fabric_server_control_comm_connect] ERROR: socket send addres fails");
    socket::close(connection_socket);
    return nullptr;
  }
  

  socket::close(connection_socket);

  if (ret < 0) {
    print("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] ERROR: Lookup "<<srv_name<<" Port "<<port_name);
    return nullptr;
  }

  debug_info("[NFI_FABRIC_SERVER_COMM] ----SERVER = "<<srv_name<<" PORT = "<<port_name);

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_connect] << End\n");

  return new (std::nothrow) nfi_fabric_server_comm(new_fabric_comm);
}

void nfi_fabric_server_control_comm::disconnect(nfi_xpn_server_comm *comm) 
{
  int ret;
  nfi_fabric_server_comm *in_comm = static_cast<nfi_fabric_server_comm*>(comm);

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] >> Begin");

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] Send disconnect message");
  ret = in_comm->write_operation(xpn_server_ops::DISCONNECT);
  if (ret < 0) {
    printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] ERROR: nfi_fabric_server_comm_write_operation fails");
  }

  // Disconnect
  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] Disconnect");

  ret = fabric::close(in_comm->m_comm);
  if (ret < 0) {
    printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] ERROR: MPI_Comm_disconnect fails");
  }

  delete comm;

  debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_disconnect] << End");
}

int64_t nfi_fabric_server_comm::write_operation(xpn_server_ops op) {
    int ret;
    int msg[2];

    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_operation] >> Begin");

    // Message generation
    msg[0] = (int)(pthread_self() % 32450) + 1;
    msg[1] = (int)op;

    // Send message
    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_operation] Write operation send tag "<< msg[0]);

    ret = fabric::send(m_comm, msg, sizeof(msg));
    if (ret < 0) {
        debug_error("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_operation] ERROR: socket::send < 0 : "<< ret);
        return -1;
    }

    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_operation] << End");

    // Return OK
    return 0;
}

int64_t nfi_fabric_server_comm::write_data(const void *data, int64_t size) {
    int ret;

    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_data] >> Begin");

    // Check params
    if (size == 0) {
        return 0;
    }
    if (size < 0) {
        printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_data] ERROR: size < 0");
        return -1;
    }

    // Send message
    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_data] Write data");

    ret = fabric::send(m_comm, data, size);
    if (ret < 0) {
        printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_data] ERROR: MPI_Send fails");
        size = 0;
    }

    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_write_data] << End");

    // Return bytes written
    return size;
}

int64_t nfi_fabric_server_comm::read_data(void *data, ssize_t size) {
    int ret;

    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_read_data] >> Begin");

    // Check params
    if (size == 0) {
        return 0;
    }
    if (size < 0) {
        printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_read_data] ERROR: size < 0");
        return -1;
    }

    // Get message
    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_read_data] Read data");

    ret = fabric::recv(m_comm, data, size);
    if (ret < 0) {
        printf("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_read_data] ERROR: MPI_Recv fails");
        size = 0;
    }

    debug_info("[NFI_FABRIC_SERVER_COMM] [nfi_fabric_server_comm_read_data] << End");

    // Return bytes read
    return size;
}

} //namespace XPN
