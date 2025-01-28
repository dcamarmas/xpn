
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

#include "base_cpp/debug.hpp"
#include "base_cpp/ns.hpp"
#include "nfi_server.hpp"
#include "xpn/xpn_api.hpp"
#include "nfi/nfi_xpn_server/nfi_xpn_server.hpp"
#include "nfi/nfi_local/nfi_local.hpp"

#include <iostream>
#include <csignal>

namespace XPN
{
    nfi_parser::nfi_parser(const std::string &url) : m_url(url)
    {
        XPN_DEBUG_BEGIN;
        int res = 0;

        std::tie(m_protocol, m_server, m_path) = parse(m_url);

        if (m_protocol.empty()){
            std::cerr << "Error cannot parse protocol of url '" << m_url << "'" << std::endl;
            std::raise(SIGTERM);
        }
        if (m_server.empty()){
            std::cerr << "Error cannot parse server of url '" << m_url << "'" << std::endl;
            std::raise(SIGTERM);
        }
        if (m_path.empty()){
            std::cerr << "Error cannot parse path of url '" << m_url << "'" << std::endl;
            std::raise(SIGTERM);
        }
        XPN_DEBUG_END;
    }

    std::tuple<std::string, std::string, std::string> nfi_parser::parse(const std::string& url){
        XPN_DEBUG_BEGIN;
        int res = 0;
        std::string protocol;
        std::string server;
        std::string path;
        // Find the position of "://"
        size_t protocol_pos = url.find("://");
        if (protocol_pos == std::string::npos) {
            std::cerr << "Invalid format of server_url: '://' not found '" << url << "'" << std::endl;
        }else{
            // Extract the first part (before "://")
            protocol = url.substr(0, protocol_pos);

            // Extract the second part (after "://")
            std::string remainder = url.substr(protocol_pos + 3);

            // Find the position of the first '/'
            size_t ip_pos = remainder.find('/');
            if (ip_pos == std::string::npos) {
                std::cerr << "Invalid format: '/' not found after IP '" << url << "'" << std::endl;
            }else{
                // Extract the IP address
                server = remainder.substr(0, ip_pos);
                // Extract the path (after the first '/')
                path = remainder.substr(ip_pos);
            }
        }

        XPN_DEBUG("Parse '"<<url<<"' to protocol '"
        << protocol <<"' server '"
        << server << "' path '"
        << path << "'");
        XPN_DEBUG_END;
        return {protocol, server, path};
    }

    std::string nfi_parser::create(const std::string& protocol, const std::string& server, const std::string& path) {
        return protocol + "://" + server + "/" + path;
    }

    std::unique_ptr<nfi_server> nfi_server::Create(const std::string &url)
    {
        nfi_parser parser(url);
        if (url.find(server_protocols::file) == 0 ||
            (xpn_env::get_instance().xpn_locality == 1 && is_local_server(parser.m_server))){
                return std::make_unique<nfi_local>(parser);
            }
        if (url.find(server_protocols::mpi_server) == 0 ||
            url.find(server_protocols::sck_server) == 0 ||
            url.find(server_protocols::fabric_server) == 0 ){
                return std::make_unique<nfi_xpn_server>(parser);
            }
        
        std::cerr << "Error: server protocol '"<< url << "' is not defined." << std::endl;
        return nullptr;
    }

    nfi_server::nfi_server(const nfi_parser &parser) : m_url(parser.m_url)
    {
        m_protocol = parser.m_protocol;
        m_server = parser.m_server;
        m_path = parser.m_path;
    }

    int nfi_server::init_comm()
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        // Init the comunication
        m_control_comm = nfi_xpn_server_control_comm::Create(m_protocol);

        // Connect to the server
        m_comm = m_control_comm->connect(m_server);
        if(m_comm){
            XPN_DEBUG("Connected successfull to "<<m_server);
        }

        if (m_comm == nullptr){
            m_error = -1;
            res = -1;
        }

        if (!xpn_env::get_instance().xpn_session_connect){
            m_control_comm->disconnect(m_comm);
            m_comm = nullptr;
        }

        XPN_DEBUG_END;
        return res;
    }

    int nfi_server::destroy_comm()
    {
        XPN_DEBUG_BEGIN;
        int res = 0;
        if (m_comm != nullptr){
            m_control_comm->disconnect(m_comm);
        }

        m_control_comm.reset();

        XPN_DEBUG_END;
        return res;
    }

    bool nfi_server::is_local_server(const std::string &server)
    {
        return (server == ns::get_host_name() ||
                server == ns::get_host_ip());
    }
} // namespace XPN
