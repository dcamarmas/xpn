
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

#include <sys/time.h>

#include "mosquitto.h"

namespace XPN {
struct xpn_server_param_st;
/* ... Functions / Funciones ......................................... */
class mqtt_server_comm {
   public:
    static void on_message(struct mosquitto* mqtt, void* obj, const struct mosquitto_message* msg);

    static int mqtt_server_mqtt_init(struct mosquitto** mqtt);
    static int mqtt_server_mqtt_destroy(struct mosquitto* mqtt);
};

/* ................................................................... */

}  // namespace XPN