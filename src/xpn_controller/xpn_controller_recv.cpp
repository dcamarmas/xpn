
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
#define DEBUG
#include <limits.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "base_cpp/subprocess.hpp"
#include "base_cpp/workers.hpp"
#include "xpn/xpn_conf.hpp"
#include "xpn_controller.hpp"

namespace XPN {

int xpn_controller::recv_action(int socket, action &act) {
    int ret;
    int code;
    debug_info("[XPN_CONTROLLER] >> Start");
    ret = socket::recv(socket, &code, sizeof(code));
    if (ret != sizeof(code)) {
        print_error("recving code");
        socket::close(socket);
        return -1;
    }
    act = static_cast<action>(code);

    switch (act) {
        case action::STOP:
            code = recv_stop(socket);
            break;
        case action::START_SERVERS:
            code = recv_start_servers(socket);
            break;
        case action::STOP_SERVERS:
            code = recv_stop_servers(socket);
            break;
        case action::PING_SERVERS:
            code = recv_ping_servers(socket);
            break;
        default:
            break;
    }

    ret = socket::send(socket, &code, sizeof(code));
    if (ret != sizeof(code)) {
        print_error("sending code to the xpn_controller");
        socket::close(socket);
        return -1;
    }
    debug_info("[XPN_CONTROLLER] >> End");
    if (code < 0) {
        return code;
    }
    return 0;
}

int xpn_controller::recv_stop(int socket) {
    debug_info("[XPN_CONTROLLER] >> Start");
    // Stop the servers to stop the controler
    int ret = recv_stop_servers(socket);
    debug_info("[XPN_CONTROLLER] >> End");
    return ret;
}

int xpn_controller::recv_mk_config(int socket) {
    // Recv the data
    std::string hostfile, conffile, bsize, replication_level, server_type, storage_path;
    int64_t ret;
    debug_info("[XPN_CONTROLLER] >> Start");
    ret = socket::recv_str(socket, hostfile);
    if (ret < 0) {
        print_error("recv_str hostfile");
        return ret;
    }
    ret = socket::recv_str(socket, conffile);
    if (ret < 0) {
        print_error("recv_str conffile");
        return ret;
    }
    ret = socket::recv_str(socket, bsize);
    if (ret < 0) {
        print_error("recv_str bsize");
        return ret;
    }
    ret = socket::recv_str(socket, replication_level);
    if (ret < 0) {
        print_error("recv_str replication_level");
        return ret;
    }
    ret = socket::recv_str(socket, server_type);
    if (ret < 0) {
        print_error("recv_str server_type");
        return ret;
    }
    ret = socket::recv_str(socket, storage_path);
    if (ret < 0) {
        print_error("recv_str storage_path");
        return ret;
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
        if (server_type.empty()) {
            server_type = XPN_CONF::DEFAULT_SERVER_TYPE;
        }
        if (storage_path.empty()) {
            storage_path = XPN_CONF::DEFAULT_STORAGE_PATH;
        }
        while (std::getline(file, line)) {
            if (!line.empty()) {
                line = server_type + "_server://" + line + "/" + storage_path;
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

int xpn_controller::recv_start_servers(int socket) {
    bool await;
    int ret;
    debug_info("[XPN_CONTROLLER] >> Start");
    ret = socket::recv(socket, &await, sizeof(await));
    if (ret != sizeof(await)) {
        print_error("recv await");
        return -1;
    }
    int server_cores = 0;
    ret = socket::recv(socket, &server_cores, sizeof(server_cores));
    if (ret != sizeof(server_cores)) {
        print_error("recv server_cores");
        return -1;
    }
    ret = start_servers(await, server_cores);
    debug_info("[XPN_CONTROLLER] >> End");
    return ret;
}

int xpn_controller::recv_stop_servers(int socket) {
    bool await;
    int ret;
    debug_info("[XPN_CONTROLLER] >> Start");
    ret = socket::recv(socket, &await, sizeof(await));
    if (ret != sizeof(await)) {
        print_error("recv await");
        return -1;
    }
    ret = stop_servers(await);
    debug_info("[XPN_CONTROLLER] >> End");
    return ret;
}

int xpn_controller::recv_command(int socket) {
    int ret;
    std::string command;
    debug_info("[XPN_CONTROLLER] >> Start");
    ret = socket::recv_str(socket, command);
    if (ret < 0) {
        print_error("recv_str failed")
        return -1;
    }
    bool await;
    ret = socket::recv(socket, &await, sizeof(await));
    if (ret < 0) {
        print_error("Cannot recv await command");
        return -1;
    }
    debug_info("Recv command: '" << command << "'");
    subprocess::process proc(command);
    proc.set_wait_on_destroy(await);
    debug_info("launched command: '" << command << "'");
    int res = 0;
    if (await){
        debug_info("Wait command: '" << command << "'");
        res = proc.wait_status();
    }

    ret = socket::send(socket, &res, sizeof(res));
    if (ret < 0) {
        print_error("Cannot send str_size");
        return -1;
    }
    debug_info("[XPN_CONTROLLER] >> End");
    return 0;
}

int xpn_controller::recv_ping_servers([[maybe_unused]] int socket) {
    int ret;
    debug_info("[XPN_CONTROLlER] >> Start");
    ret = ping_servers();
    debug_info("[XPN_CONTROLlER] >> End");
    return ret;
}
}  // namespace XPN