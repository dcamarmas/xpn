
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

#include "xpn/xpn_api.hpp"

namespace XPN {

int xpn_api::start_malleability(const char* host_list, int rank, malleability_type type) {
    int res = 0;
    XPN_DEBUG_BEGIN_CUSTOM(host_list << ", " << rank);

    res = clean_connections();
    if (res < 0) {
        return res;
    }

    if (rank == 0) {
        std::string command = "xpn_controller --host_list " + std::string(host_list);
        switch (type) {
            case malleability_type::EXPAND:
                command += " expand_change";
                break;
            case malleability_type::SHRINK:
                command += " shrink_change";
                break;
        }

        res = std::system(command.c_str());
    }

    XPN_DEBUG_END_CUSTOM(host_list << ", " << rank);
    return res;
}

int xpn_api::end_malleability(const char* host_list, int rank, [[maybe_unused]] malleability_type type) {
    int res = 0;
    XPN_DEBUG_BEGIN_CUSTOM(host_list << ", " << rank);
    res = init();
    XPN_DEBUG_END_CUSTOM(host_list << ", " << rank);
    return res;
}
}  // namespace XPN