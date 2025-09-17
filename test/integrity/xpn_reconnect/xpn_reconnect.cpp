
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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

#include "xpn_controller/xpn_controller.hpp"

extern char **environ;

#include "xpn.h"

void print_env() {
    char **env = environ;

    while (*env) {
        std::cout << *env << std::endl;
        ++env;
    }
}

int main() {
    int ret = 0;
    std::cout << "Starting" << std::endl;

    char *env_command = std::getenv("ENV_COMMAND");
    if (env_command == nullptr) {
        std::cout << "No env var in ENV_COMMAND" << std::endl;
        return 1;
    }
    std::cout << "ENV_COMMAND = " << env_command << std::endl;

    // std::cout << "Start print_env" << std::endl;
    // print_env();
    // std::cout << "End print_env" << std::endl;

    // std::cout << "Start system env" << std::endl;
    // ret = std::system("env");
    // std::cout << "End system env = " << ret << std::endl;
    std::string sleep_command = "sleep 10 &";
    std::cout << "Start send_command(" << sleep_command << ")" << std::endl;
    ret = XPN::xpn_controller::send_command(sleep_command, false);
    std::cout << "End send_command(" << sleep_command << ") = " << ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    sleep_command = "srun -n 1 -N 1 -v sleep 10 &";
    std::cout << "Start send_command(" << sleep_command << ")" << std::endl;
    ret = XPN::xpn_controller::send_command(sleep_command, false);
    std::cout << "End send_command(" << sleep_command << ") = " << ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Start xpn_init" << std::endl;
    ret = xpn_init();
    std::cout << "End xpn_init = " << ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Start xpn_destroy" << std::endl;
    ret = xpn_destroy();
    std::cout << "End xpn_destroy = " << ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Start system(" << env_command << ")" << std::endl;
    // ret = XPN::xpn_controller::send_command(env_command);
    ret = std::system(env_command);
    std::cout << "End system(" << env_command << ") = " << ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Start xpn_init" << std::endl;
    ret = xpn_init();
    std::cout << "End xpn_init = " << ret << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Start xpn_destroy" << std::endl;
    ret = xpn_destroy();
    std::cout << "End xpn_destroy = " << ret << std::endl;

    return 0;
}