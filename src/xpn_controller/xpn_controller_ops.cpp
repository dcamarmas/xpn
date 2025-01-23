
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

#include <limits.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include "base_cpp/args.hpp"
#include "base_cpp/subprocess.hpp"
#include "nfi/nfi_server.hpp"
#include "xpn/xpn_conf.hpp"
#include "xpn_controller.hpp"

namespace XPN {

std::string xpn_controller::usage() {
    std::stringstream out;
    out << "xpn_controller [OPTION]... [ACTION]..." << std::endl;
    out << "  Actions: ";
    for (auto& [key, _] : actions_str) {
        out << key << " ";
    }

    out << std::endl;
    return out.str();
}

int xpn_controller::run() {
    int ret;
    debug_info("[XPN_CONTROLLER] >> Start");

    ret = first_mk_config();
    if (ret < 0) {
        std::cerr << "Error in mk_config" << std::endl;
        return ret;
    }

    ret = start_servers(m_args.has_option(option_await));
    if (ret < 0) {
        std::cerr << "Error in start_servers" << std::endl;
        return ret;
    }

    int server_socket = -1;
    ret = socket::server_create(xpn_env::get_instance().xpn_controller_sck_port, server_socket);
    if (ret < 0) {
        print_error("starting the socket");
        return -1;
    }

    int connection_socket = -1;
    int op_code = 0;
    bool running = true;
    while (running) {
        connection_socket = -1;
        debug_info("[XPN_CONTROLLER] Accepting connections");
        ret = socket::server_accept(server_socket, connection_socket);
        if (ret < 0 || connection_socket < 0) {
            print_error("accept failed");
            continue;
        }
        debug_info("[XPN_CONTROLLER] Connection accepted");

        ret = socket::recv(connection_socket, &op_code, sizeof(op_code));
        if (ret != sizeof(op_code)) {
            print_error("accept failed");
            socket::close(connection_socket);
            continue;
        }
        debug_info("[XPN_CONTROLLER] Recieve op " << op_code);

        switch (op_code) {
            case socket::xpn_controller::COMMAND_CODE:
                ret = recv_command(connection_socket);
                break;

            case socket::xpn_controller::ACTION_CODE:
                action act;
                ret = recv_action(connection_socket, act);
                if (act == action::STOP) {
                    running = false;
                }
                break;

            default:
                print_error("Recv unknown op_code '" << op_code << "'");
                break;
        }

        socket::close(connection_socket);
    }
    debug_info("[XPN_CONTROLLER] >> End");
    return 0;
}

int xpn_controller::first_mk_config() {
    debug_info("[XPN_CONTROLLER] >> Start");
    // Necesary options
    auto hostfile = m_args.get_option(option_hostfile);
    auto conffile = xpn_env::get_instance().xpn_conf;
    // Optional options
    auto bsize = m_args.get_option(option_bsize);
    auto replication_level = m_args.get_option(option_replication_level);
    auto server_type = m_args.get_option(option_server_type);
    auto storage_path = m_args.get_option(option_storage_path);

    int ret = mk_config(hostfile, conffile, bsize, replication_level, server_type, storage_path);

    debug_info("[XPN_CONTROLLER] >> End");
    return ret;
}

int xpn_controller::mk_config(const std::string_view& hostfile, const char* conffile,
                                    const std::string_view& bsize, const std::string_view& replication_level,
                                    const std::string_view& server_type, const std::string_view& storage_path) {
    int ret;
    debug_info("[XPN_CONTROLLER] >> Start");
    if (hostfile.empty()) {
        std::cerr << "To mk_config is necesary the option hostfile" << std::endl;
        return -1;
    }
    if (conffile == nullptr) {
        std::cerr << "To mk_config is necesary to set the XPN_CONF to a shared file path" << std::endl;
        return -1;
    }

    char hostname[HOST_NAME_MAX];
    ret = gethostname(hostname, HOST_NAME_MAX);
    if (ret < 0) {
        print_error("gethostname");
    }

    xpn_conf::partition part;

    part.controler_url = hostname;
    if (!bsize.empty()) {
        part.bsize = atoi(std::string(bsize).c_str());
    }
    if (!replication_level.empty()) {
        part.replication_level = atoi(std::string(replication_level).c_str());
    }

    // Read the hosts
    {
        std::string filename(hostfile);
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: hostfile '" << filename << "' cannot be open " << std::endl;
            return -1;
        }
        std::string line;
        // Get the options or default op
        std::string server_type_str = server_type.empty() ? XPN_CONF::DEFAULT_SERVER_TYPE : std::string(server_type);
        std::string storage_path_str =
            storage_path.empty() ? XPN_CONF::DEFAULT_STORAGE_PATH : std::string(storage_path);
        while (std::getline(file, line)) {
            if (!line.empty()) {
                line = server_type_str + "_server://" + line + "/" + storage_path_str;
                part.server_urls.emplace_back(line);
            }
            line.clear();
        }
    }

    {
        std::string filename(conffile);
        std::ofstream file(filename, std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error: conffile '" << filename << "' cannot be open " << std::endl;
            return -1;
        }

        file << part.to_string();
    }

    debug_info("[XPN_CONTROLLER] >> End");
    return 0;
}

int xpn_controller::start_servers(bool await) {
    int ret;
    std::vector<std::string> servers;

    debug_info("[XPN_CONTROLLER] >> Start");

    if (get_conf_servers(servers) < 0) return -1;
    if (servers.size() == 0) return -1;

    std::string command;
    std::vector<std::string> args;
    if (subprocess::have_command("srun")) {
        command = "srun";
        args.emplace_back("-n");
        args.emplace_back(std::to_string(servers.size()));
        args.emplace_back("-N");
        args.emplace_back(std::to_string(servers.size()));
        args.emplace_back("-l");
        args.emplace_back("--export=ALL");
        args.emplace_back("--mpi=none");
        std::stringstream servers_list;
        for (size_t i = 0; i < servers.size(); i++) {
            nfi_parser parser(servers[i]);
            auto& server = parser.m_server;
            servers_list << server;
            if (i != (servers.size() - 1)) {
                servers_list << ",";
            }
            m_servers.emplace(server);
        }
        args.emplace_back("-w");
        args.emplace_back(servers_list.str());
        args.emplace_back("xpn_server");
        args.emplace_back("-s");
        nfi_parser parser(servers[0]);
        size_t pos = parser.m_protocol.find('_');
        args.emplace_back(parser.m_protocol.substr(0, pos));
        args.emplace_back("-t");
        args.emplace_back("pool");
    } else {
        // TODO: with mpiexec
        throw std::runtime_error("TODO: launch with mpiexec the servers");
    }
    #ifdef DEBUG
    std::stringstream command_str;
    command_str << command << " ";
    for (auto& arg : args){
        command_str << arg << " ";
    }
    debug_info("Command to launch: '" << command_str.str() << "'");
    #endif

    m_servers_process.execute(command, args, false);
    if (!m_servers_process.is_running()){
        return -1;
    }

    if (await) {
        ret = ping_servers();
    }
    debug_info("[XPN_CONTROLLER] >> End");
    return ret;
}

int xpn_controller::stop_servers(bool await) {
    int res = 0;
    debug_info("[XPN_CONTROLLER] >> Start");
    std::unique_ptr<workers> worker = workers::Create(workers_mode::thread_on_demand);
    std::vector<std::future<int>> v_res(m_servers.size());
    int index = 0;
    for (auto& name : m_servers) {
        v_res[index++] = worker->launch([this, &name, await]() {
            printf(" * Stopping server (%s)\n", name.c_str());
            int socket;
            int ret;
            int buffer = socket::xpn_server::FINISH_CODE;
            if (await) {
                buffer = socket::xpn_server::FINISH_CODE_AWAIT;
            }
            ret = socket::client_connect(name, xpn_env::get_instance().xpn_sck_port, socket);
            if (ret < 0) {
                print("[XPN_CONTROLLER] ERROR: socket connection " << name);
                return ret;
            }

            ret = socket::send(socket, &buffer, sizeof(buffer));
            if (ret < 0) {
                print("[XPN_CONTROLLER] ERROR: socket send " << name);
            }

            if (!await) {
                socket::close(socket);
            }

            if (await) {
                ret = socket::recv(socket, &buffer, sizeof(buffer));
                if (ret < 0) {
                    print("[XPN_CONTROLLER] ERROR: socket recv " << name);
                }
                socket::close(socket);
            }
            return ret;
        });
    }

    int aux_res;
    for (auto& fut : v_res) {
        aux_res = fut.get();
        if (aux_res < 0) {
            res = aux_res;
        }
    }

    m_servers.clear();

    debug_info("[XPN_CONTROLLER] >> End");

    return res;
}

int xpn_controller::ping_servers() {
    int res = 0;
    debug_info("[XPN_CONTROLLER] >> Start");
    std::unique_ptr<workers> worker = workers::Create(workers_mode::thread_on_demand);
    std::vector<std::future<int>> v_res(m_servers.size());
    int index = 0;
    for (auto& name : m_servers) {
        v_res[index++] = worker->launch([this, &name]() {
            printf(" * Ping server (%s)\n", name.c_str());
            int socket;
            int ret = -1;
            int buffer = socket::xpn_server::PING_CODE;
            auto start = std::chrono::high_resolution_clock::now();
            auto wait_time_ms = 5000;
            while (ret < 0) {
                debug_info("[XPN_CONTROLLER] Try to connect to " << name << " server");
                ret = socket::client_connect(name, xpn_env::get_instance().xpn_sck_port, socket);
                if (ret < 0) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - start)
                                       .count();
                    debug_info("[XPN_CONTROLLER] ERROR: failed to connect to " << name << " server. Elapsed time "
                                                                                     << elapsed << " ms");
                    if (elapsed > wait_time_ms) {
                        print("[XPN_CONTROLLER] ERROR: socket connection " << name);
                        return ret;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
            }

            ret = socket::send(socket, &buffer, sizeof(buffer));
            if (ret < 0) {
                print("[XPN_CONTROLLER] ERROR: socket send ping " << name);
            }

            ret = socket::recv(socket, &buffer, sizeof(buffer));
            if (ret < 0) {
                print("[XPN_CONTROLLER] ERROR: socket recv ping " << name);
            }
            socket::close(socket);
            return ret;
        });
    }

    int aux_res;
    for (auto& fut : v_res) {
        aux_res = fut.get();
        if (aux_res < 0) {
            res = aux_res;
        }
    }

    debug_info("[XPN_CONTROLLER] >> End");
    return res;
}
}  // namespace XPN