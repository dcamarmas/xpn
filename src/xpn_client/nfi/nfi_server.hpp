
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

#pragma once

#include <string>
#include <memory>
#include <tuple>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>

#include "xpn_server/xpn_server_ops.hpp"
#include "nfi_xpn_server_comm.hpp"
#include "base_cpp/debug.hpp"
#include "base_cpp/xpn_parser.hpp"

namespace XPN
{
    // Fordward declaration
    class xpn_fh;
    class xpn_metadata;

    class nfi_server 
    {
    public:
        nfi_server(const xpn_parser &url);
        int init_comm();
        int destroy_comm();
        static bool is_local_server(const std::string_view &server);
        
        static std::unique_ptr<nfi_server> Create(const std::string &url);
    public:
        std::string m_protocol; // protocol of the server: mpi_server sck_server
        std::string m_server;   // server address
        std::string m_path;     // path of the server

        std::string m_connectionless_port = {}; // port for the connectionless socket

        int m_error = 0;        // For fault tolerance
    protected:
        const std::string m_url;// URL of this server -> protocol
                                // + server
                                // + path + more info (port, ...)

        std::unique_ptr<nfi_xpn_server_control_comm> m_control_comm;
        std::unique_ptr<nfi_xpn_server_control_comm> m_control_comm_connectionless;
        nfi_xpn_server_comm                         *m_comm;

    public:
        // Operations 
        virtual int nfi_open        (const std::string &path, int flags, mode_t mode, xpn_fh &fho) = 0; 
        virtual int nfi_create      (const std::string &path, mode_t mode, xpn_fh &fho) = 0;
        virtual int nfi_close       (const xpn_fh &fh) = 0;
        virtual int64_t nfi_read    (const xpn_fh &fh,       char *buffer, int64_t offset, uint64_t size) = 0;
        virtual int64_t nfi_write   (const xpn_fh &fh, const char *buffer, int64_t offset, uint64_t size) = 0;
        virtual int nfi_remove      (const std::string &path, bool is_async) = 0;
        virtual int nfi_rename      (const std::string &path, const std::string &new_path) = 0;
        virtual int nfi_getattr     (const std::string &path, struct ::stat &st) = 0;
        virtual int nfi_setattr     (const std::string &path, struct ::stat &st) = 0;
        virtual int nfi_mkdir       (const std::string &path, mode_t mode) = 0;
        virtual int nfi_opendir     (const std::string &path, xpn_fh &fho) = 0;
        virtual int nfi_readdir     (xpn_fh &fhd, struct ::dirent &entry) = 0;
        virtual int nfi_closedir    (const xpn_fh &fhd) = 0;
        virtual int nfi_rmdir       (const std::string &path, bool is_async) = 0;
        virtual int nfi_statvfs     (const std::string &path, struct ::statvfs &inf) = 0;
        virtual int nfi_read_mdata  (const std::string &path, xpn_metadata &mdata) = 0;
        virtual int nfi_write_mdata (const std::string &path, const xpn_metadata::data &mdata, bool only_file_size) = 0;
    protected:
    
        template<typename msg_struct>
        int nfi_write_operation( xpn_server_ops op, msg_struct &msg, bool withoutRequest = false )
        {
            int ret;

            debug_info("[NFI_XPN] [nfi_write_operation] >> Begin");

            debug_info("[NFI_XPN] [nfi_write_operation] Send operation");

            xpn_server_msg message;
            message.op = static_cast<int>(op);
            message.msg_size = msg.get_size();
            std::memcpy(message.msg_buffer, &msg, msg.get_size());

            if (withoutRequest && !xpn_env::get_instance().xpn_connect && m_comm == nullptr){
                m_comm = m_control_comm_connectionless->connect(m_server, m_connectionless_port);
            }
            ret = m_comm->write_operation(message);
            if (ret < 0)
            {
                printf("[NFI_XPN] [nfi_write_operation] ERROR: nfi_write_operation fails");
                return -1;
            }
            if (withoutRequest && !xpn_env::get_instance().xpn_connect){
                m_control_comm_connectionless->disconnect(m_comm);
                m_comm = nullptr;
            }

            debug_info("[NFI_XPN] [nfi_write_operation] Execute operation: "<<static_cast<int>(op)<<" "<<xpn_server_ops_name(op)<<" -> "<<ret);

            debug_info("[NFI_XPN] [nfi_write_operation] >> End");

            return ret;
        }

        template<typename msg_struct, typename req_struct>
        int nfi_do_request ( xpn_server_ops op, msg_struct &msg, req_struct &req )
        {
            int64_t ret;
            debug_info("[NFI_XPN] [nfi_server_do_request] >> Begin");

            if (!xpn_env::get_instance().xpn_connect && m_comm == nullptr){
                m_comm = m_control_comm_connectionless->connect(m_server, m_connectionless_port);
            }

            // send request...
            debug_info("[NFI_XPN] [nfi_server_do_request] Send operation: "<<static_cast<int>(op)<<" "<<xpn_server_ops_name(op));

            ret = nfi_write_operation(op, msg);
            if (ret < 0) {
                debug_error("[NFI_XPN] [nfi_server_do_request] Error in nfi_write_operation");
                return -1;
            }

            // read response...
            debug_info("[NFI_XPN] [nfi_server_do_request] Response operation: "<<static_cast<int>(op)<<" "<<xpn_server_ops_name(op)<<" to read "<<sizeof(req));
            ret = m_comm->read_data((void *)&(req), sizeof(req));
            if (ret < 0) {
                return -1;
            }
            
            #ifdef DEBUG
            if constexpr (std::is_same<req_struct, st_xpn_server_status>::value) {
                debug_info("[NFI_XPN] [nfi_server_do_request] req ret "<<req.ret<<" server_errno "<<req.server_errno);
            }
            #endif

            if (!xpn_env::get_instance().xpn_connect){
                m_control_comm_connectionless->disconnect(m_comm);
                m_comm = nullptr;
            }
            debug_info("[NFI_XPN] [nfi_server_do_request] >> End");

            return 0;
        }
    };
} // namespace XPN