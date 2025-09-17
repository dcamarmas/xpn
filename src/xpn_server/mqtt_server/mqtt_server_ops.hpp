
/*
 *  Copyright 2020-2025 Felix Garcia Carballeira, Diego Camarmas Alonso, Elias Del Pozo Puñal, Alejandro Calderon
 * Mateos, Dario Muñoz Muñoz
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

/* ... Include / Inclusion ........................................... */

#include "mosquitto.h"

/* ... Functions / Funciones ......................................... */

namespace XPN {
class mqtt_server_ops {
   public:
    static void subscribe(struct mosquitto *mqtt, int mosquitto_qos, const char *path);
    static void unsubscribe(struct mosquitto *mqtt, const char *path);
};

/* ................................................................... */

}  // namespace XPN