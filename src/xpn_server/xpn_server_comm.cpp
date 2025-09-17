
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

/* ... Include / Inclusion ........................................... */

#include "xpn_server_comm.hpp"
#include "sck_server/sck_server_comm.hpp"
#ifdef ENABLE_MPI_SERVER
#include "mpi_server/mpi_server_comm.hpp"
#endif
#ifdef ENABLE_FABRIC_SERVER
#include "fabric_server/fabric_server_comm.hpp"
#endif

namespace XPN {
std::unique_ptr<xpn_server_control_comm> xpn_server_control_comm::Create(xpn_server_params &params) {
    switch (params.srv_type) {
#ifdef ENABLE_MPI_SERVER
        case server_type::MPI:
            return std::make_unique<mpi_server_control_comm>(params);
#endif
        case server_type::SCK:
        case server_type::MQTT:
            return std::make_unique<sck_server_control_comm>(params, params.srv_comm_port);
#ifdef ENABLE_FABRIC_SERVER
        case server_type::FABRIC:
            return std::make_unique<fabric_server_control_comm>();
#endif
        default:
            fprintf(stderr, "[XPN_SERVER] [xpn_server_control_comm] server_type '%d' not recognized\n",
                    static_cast<int>(params.srv_type));
    }
    return nullptr;
}

}  // namespace XPN
