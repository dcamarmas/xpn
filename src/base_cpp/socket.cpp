
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

#include "socket.hpp"
#include "xpn_env.hpp"
#include "debug.hpp"
#include "base_c/filesystem.h"

#include <string>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>
#include <thread>

namespace XPN
{
    int64_t socket::send ( int socket, const void * buffer, size_t size )
    {
        int64_t ret;

        ret = filesystem_write(socket, buffer, size);
        if (ret < 0){
            debug_error_f("[SOCKET] [socket::recv] ERROR: socket read buffer size %ld Failed\n", size);
        }
        
        return size;
    }

    int64_t socket::recv ( int socket, void * buffer, size_t size )
    {
        int64_t ret;

        ret = filesystem_read(socket, buffer, size);
        if (ret < 0){
            debug_error_f("[SOCKET] [socket::recv] ERROR: socket read buffer size %ld Failed\n", size);
        }

        return size;
    }
    
    int64_t socket::send_line ( int socket, const char *buffer )
    {
        return socket::send(socket, buffer, strlen(buffer)+1);
    }

    int64_t socket::recv_line ( int socket, char *buffer, size_t n )
    {
        int64_t numRead;  /* bytes read in the last read() */
        size_t totRead;   /* total bytes read so far */
        char *buf;
        char ch;

        if (n <= 0 || buffer == NULL) {
            errno = EINVAL;
            return -1;
        }

        buf = buffer;
        totRead = 0;

        while (1)
        {
            numRead = socket::recv(socket, &ch, 1);  /* read one byte */

            if (numRead == -1) {
                if (errno == EINTR)      /* interrupted -> restart read() */
                    continue;
                else return -1;          /* another type of error */
            } else if (numRead == 0) {   /* EOF */
                if (totRead == 0)        /* no bytes read -> return 0 */
                    return 0;
                else break;
            } else {                     /* numRead must be 1 here */
                if (ch == '\n') break;
                if (ch == '\0') break;
                if (totRead < n - 1) {   /* discard > (n-1) bytes */
                    totRead++;
                    *buf++ = ch;
                }
            }
        }

        *buf = '\0';
        return totRead;
    }

    int64_t socket::send_str ( int socket, const std::string& str )
    {
        return socket::send_str(socket, std::string_view(str));
    }

    int64_t socket::send_str ( int socket, const std::string_view& str )
    {
        int64_t ret;
        size_t size_str = str.size();
        debug_info("Send_str size "<<size_str);
        ret = socket::send(socket, &size_str, sizeof(size_str));
        if (ret != sizeof(size_str)){
            print_error("send size of string");
            return ret;
        }
        if (size_str == 0){
            return 0;
        }
        debug_info("Send_str "<<str);
        ret = socket::send(socket, &str[0], size_str);
        if (ret != static_cast<int64_t>(size_str)){
            print_error("send string");
            return ret;
        }
        return ret;
    }

    int64_t socket::recv_str ( int socket, std::string& str )
    {
        int64_t ret;
        size_t size_str = 0;
        ret = socket::recv(socket, &size_str, sizeof(size_str));
        if (ret != sizeof(size_str)){
            print_error("send size of string");
            return ret;
        }
        debug_info("Recv_str size "<<size_str);
        if (size_str == 0){
            return 0;
        }
        str.clear();
        str.resize(size_str, '\0');
        ret = socket::recv(socket, &str[0], size_str);
        if (ret != static_cast<int64_t>(size_str)){
            print_error("send string");
            return ret;
        }
        debug_info("Recv_str "<<str);
        return ret;
    }

    int socket::server_create ( int port, int &out_socket )
    {
        int ret = 0;
        int server_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket < 0)
        {
            debug_error_f("[SOCKET] [socket::server_create] ERROR: socket fails\n");
            return -1;
        }


        int val = 1;
        ret = setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
        if (ret < 0)
        {
            debug_error_f("[SOCKET] [socket::server_create] ERROR: setsockopt fails\n");
            return -1;
        }

        debug_info_f("[SOCKET] [socket::server_create] Socket reuseaddr\n");

        val = 1;
        ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int));
        if (ret < 0)
        {
            debug_error_f("[SOCKET] [socket::server_create] ERROR: setsockopt fails\n");
            return -1;
        }

        // bind
        debug_info_f("[SOCKET] [socket::server_create] Socket bind\n");

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family      = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port        = htons(port);


        ret = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret < 0)
        {
            debug_error_f("[SOCKET] [socket::server_create] ERROR: bind fails\n");
            return -1;
        }

        // listen
        debug_info_f("[SOCKET] [socket::server_create] Socket listen\n");

        ret = listen(server_socket, SOMAXCONN);
        if (ret < 0)
        {
            debug_error_f("[SOCKET] [socket::server_create] ERROR: listen fails\n");
            return -1;
        }
        out_socket = server_socket;
        return 0;
    }

    int socket::server_accept ( int socket, int &out_conection_socket )
    {
        struct sockaddr_in client_addr;
        socklen_t sock_size = sizeof(sockaddr_in);
        int new_socket = accept(socket, (struct sockaddr*)&client_addr, &sock_size);
        if (new_socket < 0) {
            debug_error_f("[SOCKET] [socket::accept_send] ERROR: socket accept\n");
            return -1;
        }
        out_conection_socket = new_socket;
        return 0;
    }

    int resolve_hostname(const std::string &srv_name, sockaddr_in* pAddr)
    {
        int ret;
        addrinfo* pResultList = NULL;
        addrinfo hints = {};
        int result = -1;

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        ret = ::getaddrinfo(srv_name.c_str(), NULL, &hints, &pResultList);

        result = (ret == 0) ? 1 : -1;
        if (result != -1)
        {
            // just pick the first one found
            *pAddr = *(sockaddr_in*)(pResultList->ai_addr);
            result = 0;
        }

        if (pResultList != NULL)
        {
            ::freeaddrinfo(pResultList);
        }

        return result;
    }

    int socket::client_connect ( const std::string &srv_name, int port, int &out_socket )
    {
        int client_fd;
        struct sockaddr_in serv_addr;
        client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) 
        {
            debug_error_f("[SOCKET] [socket::read] ERROR: socket creation error\n");
            return -1;
        }
        resolve_hostname(srv_name, &serv_addr);
        serv_addr.sin_port = htons(port);
        int status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (status < 0) 
        {
            debug_error_f("[SOCKET] [socket::read] ERROR: socket connection failed to %s in port %d %s\n", srv_name.c_str(), xpn_env::get_instance().xpn_sck_port, strerror(errno));
            close(client_fd);
            return -1;
        }

        out_socket = client_fd;
        return 0;
    }

    int socket::client_connect ( const std::string &srv_name, int port, int timeout_ms, int &out_socket, int time_to_sleep_ms )
    {
        int ret = -1;
        auto start = std::chrono::high_resolution_clock::now();
        while (ret < 0) {
            debug_info("Try to connect to " << srv_name << " server");
            ret = socket::client_connect(srv_name, port, out_socket);
            if (ret < 0) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::high_resolution_clock::now() - start)
                                    .count();
                debug_error("Failed to connect to " << srv_name << " server. Elapsed time " << elapsed << " ms");
                if (elapsed > timeout_ms) {
                    debug_error("Socket connection " << srv_name);
                    return ret;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(time_to_sleep_ms));
            }
        }
        return ret;
    }

    int socket::close ( int socket )
    {
        int ret;

        ret = ::close(socket);
        if (ret < 0) {
            debug_error_f("[SOCKET] [socket::socket_close] ERROR: socket close Failed\n");
            return -1;
        }

        return ret;
    }
} // namespace XPN
