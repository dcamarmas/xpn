
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

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>

#include <mutex>

namespace XPN {

class fabric {
public:
    struct domain {
        struct fi_info *hints, *info;
        struct fid_fabric *fabric;
        struct fid_domain *domain;
    };

    struct comm {
        struct domain *fabric_domain;
        struct fid_ep *ep;
        struct fid_av *av;
        struct fid_cq *cq;
        fi_addr_t fi_addr;
    };

public:
    static int init(domain &fabric);

    static int new_comm(domain &domain, comm &out_fabric_comm);

    static int get_addr(comm &fabric_comm, char *out_addr, size_t &size_addr);
    static int register_addr(comm &fabric_comm, char *addr_buf);
    static int wait(comm &fabric_comm);
    static int send(comm &fabric, const void *buffer, size_t size);
    static int recv(comm &fabric, void *buffer, size_t size);
    static int close(comm &fabric);
    static int destroy(domain &domain);

    static std::mutex s_mutex;
};

}  // namespace XPN