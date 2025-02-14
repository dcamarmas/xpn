
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

#include <unistd.h>
#include <base_cpp/proxy.hpp>
#include <base_cpp/debug.hpp>

namespace XPN {

class filesystem {
   public:
    static ssize_t write(int fd, const void* data, size_t len) {
        int r;
        int l = len;
        const char* buffer = static_cast<const char*>(data);
        debug_info(">> Begin");

        do {
            r = PROXY(write)(fd, buffer, l);
            if (r < 0) return r; /* fail */

            l = l - r;
            buffer = buffer + r;

        } while ((l > 0) && (r >= 0));

        debug_info(">> End = " << len);
        return len;
    }
    static ssize_t read(int fd, void* data, size_t len) {
        int r;
        int l = len;
        debug_info(">> Begin");
        char* buffer = static_cast<char*>(data);

        do {
            r = PROXY(read)(fd, buffer, l);
            if (r < 0) return r; /* fail */

            l = l - r;
            buffer = buffer + r;

        } while ((l > 0) && (r >= 0));

        debug_info(">> End = " << len);
        return len;
    }
};
}  // namespace XPN