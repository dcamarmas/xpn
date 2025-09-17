
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

/* ... Include / Inclusion ........................................... */

#include "mq_server_ops.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base_cpp/debug.hpp"

namespace XPN {
/* ... Functions / Funciones ......................................... */

void mq_server_ops::subscribe(struct mosquitto *mqtt, int mosquitto_qos, const char *path) {
    // char * s;
    const char *extra = "/#";
    char *sm = (char *)malloc(strlen(path) + strlen(extra) + 1);
    strcpy(sm, path);
    strcat(sm, extra);

    debug_info("BEGIN OPEN MOSQUITTO MQ_SERVER WS - " << sm);

    debug_info("mosquitto_subscribe(" << mqtt << ", " << (void *)NULL << ", " << sm << ", " << mosquitto_qos << ")");
    int rc = mosquitto_subscribe(mqtt, NULL, sm, mosquitto_qos);
    if (rc != MOSQ_ERR_SUCCESS) {
        debug_error("Error subscribing open: " << mosquitto_strerror(rc));
        mosquitto_disconnect(mqtt);
    }

    debug_info("END OPEN MOSQUITTO MQ_SERVER WS - " << sm);
}

void mq_server_ops::unsubscribe(struct mosquitto *mqtt, const char *path) {
    const char *extra = "/#";
    char *sm = (char *)malloc(strlen(path) + strlen(extra) + 1);
    strcpy(sm, path);
    strcat(sm, extra);

    debug_info("BEGIN CLOSE MOSQUITTO MQ_SERVER - WS ");

    debug_info("mosquitto_unsubscribe(" << mqtt << ", " << (void *)NULL << ", " << sm << ")");
    mosquitto_unsubscribe(mqtt, NULL, sm);
    debug_info("mosquitto_unsubscribe(" << mqtt << ", " << (void *)NULL << ", " << path << ")");
    mosquitto_unsubscribe(mqtt, NULL, path);

    debug_info("END CLOSE MOSQUITTO MQ_SERVER - WS " << sm);
}

/* ................................................................... */

}  // namespace XPN