
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

#include "sck_server_comm.hpp"
#include "base_cpp/debug.hpp"
#include "base_cpp/timer.hpp"
#include "base_cpp/ns.hpp"
#include "base_cpp/socket.hpp"
#include <csignal>

#ifdef ENABLE_MQ_SERVER
#include "../mq_server/mq_server_comm.hpp"
#endif
#include <sys/epoll.h>

namespace XPN
{
sck_server_control_comm::sck_server_control_comm (int port)
{
  int ret;

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] >> Begin");

  // Get timestap
  timer timer;

  // Socket init
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] Scoket init");

  ret = socket::server_create(port, m_socket);
  if (ret < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] ERROR: socket server_create fails");
    std::raise(SIGTERM);
  }
  ret = socket::server_port(m_socket);
  if (ret < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] ERROR: socket server_port fails");
    std::raise(SIGTERM);
  }
  m_port_name = std::to_string(ret);

#ifdef ENABLE_MQ_SERVER
  if (xpn_env::get_instance().xpn_mqtt) {
    mq_server_comm::mq_server_mqtt_init(static_cast<mosquitto*>(mqtt));
  }
#endif

  m_epoll = epoll_create1(0);
  if (m_epoll == -1) {
    print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] ERROR: epoll cannot create "<<strerror(errno));
    std::raise(SIGTERM);
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] available at "<<m_port_name);
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] accepting...");

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_init] >> End");
}

sck_server_control_comm::~sck_server_control_comm()
{
#ifdef ENABLE_MQ_SERVER
  if (xpn_env::get_instance().xpn_mqtt) {
    mq_server_comm::mq_server_mqtt_destroy(static_cast<mosquitto*>(mqtt));
  }
#endif
  socket::close(m_socket);
}

xpn_server_comm* sck_server_control_comm::accept ( int socket, bool sendData )
{
  int    ret, sc, flag;
  struct sockaddr_in client_addr;
  socklen_t size = sizeof(struct sockaddr_in);

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_accept] >> Begin");
  if (sendData) {
    debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_accept] send port");
    ret = socket::send(socket, m_port_name.data(), MAX_PORT_NAME);
    if (ret < 0){
      print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_accept] ERROR: socket send port fails");
      return nullptr;
    }
  }

  // Accept
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_accept] Accept");

  sc = ::accept(m_socket, (struct sockaddr *)&client_addr, &size);
  if (sc < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_destroy] ERROR: accept fails");
    return nullptr;
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_destroy] desp. accept conection from "<<sc);
  // tcp_nodelay
  flag = 1;
  ret = ::setsockopt(sc, IPPROTO_TCP, TCP_NODELAY, & flag, sizeof(flag));
  if (ret < 0)
  {
    perror("setsockopt: ");
    return nullptr;
  }

  //NEW
  int val = MAX_BUFFER_SIZE; //1 MB
  ret = ::setsockopt(sc, SOL_SOCKET, SO_SNDBUF, (char *)&val, sizeof(int));
  if (ret < 0)
  {
    perror("setsockopt: ");
    return nullptr;
  }

  val = MAX_BUFFER_SIZE; //1 MB
  ret = ::setsockopt(sc, SOL_SOCKET, SO_RCVBUF, (char *)&val, sizeof(int));
  if (ret < 0)
  {
    perror("setsockopt: ");
    return nullptr;
  }

  struct epoll_event event;
  event.events = EPOLLIN | EPOLLONESHOT;
  event.data.fd = sc;

  if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, sc, &event) == -1) {
    debug_error("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_destroy] Error: epoll_ctl fails "<<strerror(errno));
    return nullptr;
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_accept] << End");

  return new (std::nothrow) sck_server_comm(sc);
}

int sck_server_control_comm::rearm(int socket) {
  
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLONESHOT;
  event.data.fd = socket;

  if (epoll_ctl(m_epoll, EPOLL_CTL_MOD, socket, &event) == -1) {
    debug_error("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_CONTROL_COMM] [sck_server_control_comm_destroy] Error: epoll_ctl fails "<<strerror(errno));
    return -1;
  }
  return 0;
}

void sck_server_control_comm::disconnect ( xpn_server_comm* comm )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_disconnect] >> Begin");
  
  sck_server_comm *in_comm = static_cast<sck_server_comm*>(comm);

  if (epoll_ctl(m_epoll, EPOLL_CTL_DEL, in_comm->m_socket, NULL) == -1){
    debug_error("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_disconnect] Error: epoll_ctl "<<strerror(errno));
  }

  socket::close(in_comm->m_socket);

  delete comm;

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_disconnect] << End");
}

void sck_server_control_comm::disconnect ( int socket )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_disconnect] >> Begin");
  
  socket::close(socket);

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_disconnect] << End");
}

xpn_server_comm* sck_server_control_comm::create ( int rank_client_id ) {
  return new (std::nothrow) sck_server_comm(rank_client_id);
}

int64_t sck_read_operation ( int socket, xpn_server_msg &msg, int &tag_client_id )
{
  int ret;
  uint64_t received = 0;
  char * msg_p = reinterpret_cast<char*>(&msg);

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] >> Begin");

  // Get message
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] Read operation");

  // Receive the head
  while(received < msg.get_header_size()) {
    ret = PROXY(read)(socket, msg_p+received, msg.get_header_size()-received);
    if (ret <= 0) {
      debug_warning("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] ERROR: read fails");
      return -1;
    }
    received += ret;
    debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] received "<<received<<" msg header size "<<msg.get_header_size());
  }
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] readed the msg header");
  
  // Receive the rest of the msg
  while (received < msg.get_size()) {
    ret = PROXY(read)(socket, msg_p+received, msg.get_size()-received);
    if (ret <= 0) {
      debug_warning("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] ERROR: read fails");
      return -1;
    }
    received += ret;
    debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] received "<<received<<" msg size "<<msg.get_size());
  }
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] readed the msg");
  
  tag_client_id  = msg.tag;

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] read (SOURCE "<<socket<<", TAG "<<tag_client_id<<", ERROR "<<ret<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_read_operation] << End");

  // Return OK
  return 0;
}

int64_t sck_server_control_comm::read_operation ( xpn_server_msg &msg, int &rank_client_id, int &tag_client_id )
{
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_control_comm_read_operation] >> Begin");
  struct epoll_event event;

  int nfds = epoll_wait(m_epoll, &event, 1, -1);
  if (nfds == -1) {
    debug_error("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_control_comm_read_operation] Error epoll_wait "<<strerror(errno));
    return -1;
  }

  int socket = event.data.fd;
  rank_client_id = socket;
  auto ret = sck_read_operation(socket, msg, tag_client_id);

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_control_comm_read_operation] << End");

  return ret;
}

int64_t sck_server_comm::read_operation ( xpn_server_msg &msg, int &rank_client_id, int &tag_client_id )
{
  rank_client_id = m_socket;
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm::read_operation] >> Begin");
  auto ret = sck_read_operation(m_socket, msg, tag_client_id);
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm::read_operation] << End");
  return ret;
}

int64_t sck_server_comm::read_data ( void *data, int64_t size, [[maybe_unused]] int rank_client_id, [[maybe_unused]] int tag_client_id )
{
  int ret;

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_read_data] >> Begin");

  if (size == 0) {
    return  0;
  }
  if (size < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_read_data] ERROR: size < 0");
    return  -1;
  }

  // Get message
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_read_data] Read data tag "<<tag_client_id<<" size "<<size);

  ret = socket::recv(m_socket, data, size);
  if (ret <= 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_read_data] ERROR: read fails");
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_read_data] read (SOURCE "<<m_socket<<", MPI_TAG "<<tag_client_id<<", MPI_ERROR "<<ret<<")");
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_read_data] << End");

  // Return bytes read
  return size;
}

int64_t sck_server_comm::write_data ( const void *data, int64_t size, [[maybe_unused]] int rank_client_id, [[maybe_unused]] int tag_client_id )
{
  int ret;

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_write_data] >> Begin");

  if (size == 0) {
      return 0;
  }
  if (size < 0)
  {
    print("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_write_data] ERROR: size < 0");
    return -1;
  }

  // Send message
  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_write_data] Write data tag "<<tag_client_id<<" size "<<size);

  ret = socket::send(m_socket, data, size);
  if (ret <= 0) {
    debug_warning("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_write_data] ERROR: MPI_Send fails");
  }

  debug_info("[Server="<<ns::get_host_name()<<"] [SCK_SERVER_COMM] [sck_server_comm_write_data] << End");

  // Return bytes written
  return size;
}

} // namespace XPN