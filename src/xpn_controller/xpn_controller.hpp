
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
#include <unordered_set>
#include <vector>

#include "base_cpp/args.hpp"
#include "base_cpp/debug.hpp"
#include "base_cpp/socket.hpp"
#include "base_cpp/subprocess.hpp"
#include "xpn/xpn_conf.hpp"

namespace XPN {
class xpn_controller {
   public:
    enum class action {
        NONE, // Default. Error
        MK_CONFIG,
        START,
        STOP,
        PING_SERVERS,
        START_SERVERS,
        STOP_SERVERS,
        EXPAND,
        SHRINK,
    };
    std::vector<std::pair<std::string, action>> actions_str = {
        {"mk_config",       action::MK_CONFIG},
        {"start",           action::START},
        {"stop",            action::STOP},
        {"ping_servers",    action::PING_SERVERS},
        {"start_servers",   action::START_SERVERS},
        {"stop_servers",    action::STOP_SERVERS},
        {"expand",          action::EXPAND},
        {"shrink",          action::SHRINK},
    };
    std::unordered_map<action, std::string> actions_str_help = {
        {action::MK_CONFIG,     "Make the necesary config file. Necesary before the start"},
        {action::START,         "Start the controler and servers. Necesary to run as daemon or in background '&'"},
        {action::STOP,          "Send the stop to the controler"},
        {action::PING_SERVERS,  "Send the ping of the servers to the controler to check correct status of servers"},
        {action::START_SERVERS, "Send the start of the servers to the controler"},
        {action::STOP_SERVERS,  "Send the stop of the servers to the controler"},
        {action::EXPAND,        "Send the expand of the servers to the controler"},
        {action::SHRINK,        "Send the shrink of the servers to the controler"},
    };

   public:
    xpn_controller(int argc, char* argv[]) : m_args(argc, argv, m_options, usage()) {}
    int start();

   public:
    std::unordered_set<std::string> m_servers;
    subprocess::process m_servers_process;

    // Options
    const args::option option_hostfile          {"-f", "--hostfile"         , "Hostfile with one line per host"                 , XPN::args::option::opt_type::value};
    const args::option option_bsize             {"-b", "--block_size"       , "Block size to use in the XPN partition"          , XPN::args::option::opt_type::value};
    const args::option option_replication_level {"-r", "--replication_level", "Replication level to use in the XPN partition"   , XPN::args::option::opt_type::value};
    const args::option option_server_type       {"-t", "--server_type"      , "Server type: mpi, fabric, sck"                   , XPN::args::option::opt_type::value};
    const args::option option_storage_path      {"-p", "--storage_path"     , "Storage path for the servers in local storage"   , XPN::args::option::opt_type::value};
    const args::option option_await             {"-w", "--await"            , "Await in the stop_servers"                       , XPN::args::option::opt_type::flag};
    const args::option option_server_cores      {"-c", "--server_cores"     , "Number of cores each server use (default: all cores with overlap)", XPN::args::option::opt_type::value};
    const args::option option_shared_dir        {"-s", "--shared_dir"       , "Shared dir in the job to use as temporal storage", XPN::args::option::opt_type::value};
    const std::vector<XPN::args::option> m_options = {
        option_hostfile,
        option_bsize,
        option_replication_level,
        option_server_type,
        option_storage_path,
        option_await,
        option_server_cores,
        option_shared_dir,
    };
    args m_args;

   public:
    // ops
    std::string usage();
    int run();
    int local_mk_config();
    int mk_config(const std::string_view& hostfile, const char* conffile, const std::string_view& bsize,
                  const std::string_view& replication_level, const std::string_view& server_type,
                  const std::string_view& storage_path);
    int start_servers(bool await, int server_cores);
    int stop_servers(bool await);
    int ping_servers();

   public:
    // send ops
    int send_action(action act);
    int send_stop(int socket);
    int send_mk_config(int socket);
    int send_start_servers(int socket);
    int send_stop_servers(int socket);
    int send_ping_servers(int socket);

   public:
    // recv ops
    int recv_action(int socket, action& act);
    int recv_stop(int socket);
    int recv_command(int socket);
    int recv_mk_config(int socket);
    int recv_start_servers(int socket);
    int recv_stop_servers(int socket);
    int recv_ping_servers(int socket);

   public:
    static int get_conf_servers(std::vector<std::string>& out_servers) {
        debug_info("[XPN_CONTROLLER] >> Start");
        xpn_conf conf;
        // TODO: do for more than the first partition
        out_servers.assign(conf.partitions[0].server_urls.begin(), conf.partitions[0].server_urls.end());
        debug_info("[XPN_CONTROLLER] >> End");
        return 0;
    }

    static std::string get_controler_url() {
        xpn_conf conf;
        // TODO: do for more than the first partition
        return conf.partitions[0].controler_url;
    }

    static int send_command(const std::string& command, bool await = true) {
        int socket = -1;
        int ret = -1;
        debug_info("[XPN_CONTROLLER] >> Start");
        std::string controler_url = get_controler_url();
        // TODO: do for more than the first partition
        ret = socket::client_connect(controler_url, xpn_env::get_instance().xpn_controller_sck_port, socket);
        if (ret < 0 || socket < 0) {
            print_error("Cannot connect to '" << controler_url << "'");
            return -1;
        }

        int op = socket::xpn_controller::COMMAND_CODE;
        ret = socket::send(socket, &op, sizeof(op));
        if (ret != sizeof(op)) {
            print_error("Cannot send command_code");
            socket::close(socket);
            return -1;
        }

        debug_info("Send command '" << command << "'");
        ret = socket::send_str(socket, command);
        if (ret < 0) {
            print_error("Cannot send command");
            socket::close(socket);
            return -1;
        }
        ret = socket::send(socket, &await, sizeof(await));
        if (ret < 0) {
            print_error("Cannot send await command");
            socket::close(socket);
            return -1;
        }

        int ret_command = 0;
        ret = socket::recv(socket, &ret_command, sizeof(ret_command));
        if (ret < 0) {
            print_error("Cannot recv str_size");
            socket::close(socket);
            return -1;
        }

        socket::close(socket);
        debug_info("[XPN_CONTROLLER] >> End");
        return ret_command;
    }
};
}  // namespace XPN