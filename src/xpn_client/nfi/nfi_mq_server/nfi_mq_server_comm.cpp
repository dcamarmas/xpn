
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

#include "nfi_mq_server_comm.hpp"

#include "base_cpp/debug.hpp"
#include "base_cpp/xpn_env.hpp"
#include "xpn_server/xpn_server_params.hpp"

/* ... Functions / Funciones ......................................... */

namespace XPN {
void nfi_mq_server::init(mosquitto **mqtt, const std::string &srv_name) {
    /*INIT MOSQUITTO CLIENT SIDE */
    int rc = 0;
    debug_info("BEGIN INIT MOSQUITTO NFI_MQ_SERVER");

    // MQTT initialization
    mosquitto_lib_init();

    *mqtt = mosquitto_new(NULL, true, NULL);
    if (*mqtt == NULL) {
        fprintf(stderr, "Error: Out of memory.\n");
        return;
    }

    mosquitto_int_option(*mqtt, mosq_opt_t::MOSQ_OPT_TCP_NODELAY, 1);
    mosquitto_int_option(*mqtt, mosq_opt_t::MOSQ_OPT_SEND_MAXIMUM, 65535);

    // printf("%s\n", server_aux->srv_name);
    rc = mosquitto_connect(*mqtt, srv_name.c_str(), 1883, 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(*mqtt);
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return;
    }

    /* Run the network loop in a background thread, this call returns quickly. */
    rc = mosquitto_loop_start(*mqtt);
    if (rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(*mqtt);
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return;
    }
    debug_info("END INIT MOSQUITTO NFI_MQ_SERVER");
}

void nfi_mq_server::destroy(mosquitto *mqtt) {
    debug_info("BEGIN DESTROY MOSQUITTO NFI_MQ_SERVER");
    mosquitto_disconnect(mqtt);
    mosquitto_destroy(mqtt);
    mosquitto_lib_cleanup();
    debug_info("END DESTROY MOSQUITTO NFI_MQ_SERVER");
}

int64_t nfi_mq_server::publish(mosquitto *mqtt, const char *path, const char *buffer, int64_t offset, uint64_t size) {
    int ret, diff, cont;

    ret = -1;
    diff = size;
    cont = 0;

    int buffer_size = size;

    // Max buffer size
    if (buffer_size > MAX_BUFFER_SIZE) {
        buffer_size = MAX_BUFFER_SIZE;
    }

    // writes n times: number of bytes + write data (n bytes)
    do {
        int bytes_to_write = 0;
        char *topic = (char *)malloc(strlen(path) + sizeof(bytes_to_write) + sizeof(offset) + 3);

        if (diff > buffer_size)
            bytes_to_write = buffer_size;
        else
            bytes_to_write = diff;

        sprintf(topic, "%s/%d/%ld", path, bytes_to_write, offset);

        debug_info("mosquitto_publish(" << mqtt << ", " << (void *)NULL << ", " << topic << ", " << bytes_to_write
                                        << ", " << (void *)(buffer + cont) << ", " << xpn_env::get_instance().xpn_mqtt_qos
                                        << ", " << false << ")");
        ret = mosquitto_publish(mqtt, NULL, topic, bytes_to_write, (char *)buffer + cont,
                                xpn_env::get_instance().xpn_mqtt_qos, false);
        if (ret != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error publishing write: %s\n", mosquitto_strerror(ret));
            free(topic);
            return -1;
        }

        // printf("PUBLISH --------------- topic: %s\n", topic);

        if (ret < 0) {
            fprintf(stderr, "(2)ERROR: nfi_mq_server_write: Error on write operation\n");
            return -1;
        }

        free(topic);
        cont = cont + bytes_to_write;  // Send bytes
        diff = size - cont;

    } while ((diff > 0) && (ret != 0));

    ret = cont;

    return ret;
}

/* ................................................................... */

}  // namespace XPN