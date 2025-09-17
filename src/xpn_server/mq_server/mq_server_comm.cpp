
/*
 *  Copyright 2020-2025 Felix Garcia Carballeira, Diego Camarmas Alonso, Elias del Pozo Puñal, Alejandro Calderon
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

#include "mq_server_comm.hpp"

#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../xpn_server_params.hpp"
#include "base_cpp/debug.hpp"
#include "base_cpp/filesystem.hpp"
#include "mq_server_utils.hpp"

namespace XPN {
// MOSQUITTO FILE

void *process_message([[maybe_unused]] void *arg) {
    while (1) {
        ThreadData *thread_data = mq_server_utils::dequeue_mq();
        // struct ThreadData * thread_data = (struct ThreadData *) data;

        // Copiar el mensaje en una variable local para manipularla
        char topic[PATH_MAX], path[PATH_MAX + 1];
        bzero(topic, PATH_MAX);
        bzero(path, PATH_MAX);

        int to_write1, offset;

        strncpy(topic, thread_data->topic, PATH_MAX - 1);
        debug_info("process_message from topic " << topic);
        // Encontrar la posición del último y el penúltimo slash
        int last_slash = -1;
        int penultimate_slash = -1;
        for (int i = 0; topic[i] != '\0'; i++) {
            if (topic[i] == '/') {
                penultimate_slash = last_slash;
                last_slash = i;
            }
        }

        // Extraer el path y los dos enteros usando sscanf y las posiciones de los slashes

        if (penultimate_slash >= 0 && last_slash > penultimate_slash) {
            // Si hay dos slashes, extraer el path y ambos enteros
            strncpy(path, topic, penultimate_slash);
            path[penultimate_slash] = '\0';
            sscanf(&topic[penultimate_slash + 1], "%d/%d", &to_write1, &offset);

        } else if (last_slash >= 0) {
            // Si solo hay un slash, extraer solo el path y el primer entero
            strncpy(path, topic, last_slash);
            path[last_slash] = '\0';
            sscanf(&topic[last_slash + 1], "%d", &to_write1);
            offset = 0;

        } else {
            // Si no hay slashes, asumir que todo es el path
            strncpy(path, topic, PATH_MAX);
            path[PATH_MAX - 1] = '\0';
            to_write1 = 0;
            offset = 0;
        }

        debug_info(topic << " - " << path << " " << to_write1 << " " << offset);

        // char * buffer = NULL;
        int size, diff, cont = 0, to_write = 0, size_written = 0;

        // initialize counters
        size = to_write1;
        if (size > MAX_BUFFER_SIZE) {
            size = MAX_BUFFER_SIZE;
        }
        diff = size - cont;
        // Open file
        int fd = open(path, O_WRONLY | O_CREAT, 0700);
        if (fd < 0) {
            perror("open: ");
            pthread_exit(NULL);
        }

        if (diff > size)
            to_write = size;
        else
            to_write = diff;

        size_written = filesystem::pwrite(fd, thread_data->msg, to_write, offset + cont);
        // debug_info("to write: %d\t msg: %s", to_write, thread_data -> msg);
        if (size_written < 0) {
            printf("process_message: filesystem_write return error\n");
        }

        close(fd);

        // Liberar memoria y finalizar el hilo
        free(thread_data->msg);
        free(thread_data->topic);
        free(thread_data);
    }
    pthread_exit(NULL);
}

void mq_server_comm::on_message([[maybe_unused]] struct mosquitto *mqtt, void *obj,
                                const struct mosquitto_message *msg) {
    if (NULL == obj) {
        debug_info("ERROR: obj is NULL :-( \n");
    }
    debug_info("receive " << msg->topic);
    ThreadData *thread_data = (ThreadData *)malloc(sizeof(ThreadData));

    thread_data->topic = strdup(msg->topic);
    thread_data->msg = (char *)malloc(msg->payloadlen + 1);
    bzero(thread_data->msg, msg->payloadlen + 1);
    memcpy(thread_data->msg, msg->payload, msg->payloadlen);
    thread_data->msg[msg->payloadlen] = '\0';

    mq_server_utils::enqueue_mq(thread_data);
}

int mq_server_comm::mq_server_mqtt_init(struct mosquitto **mqtt) {
    debug_info("BEGIN INIT MOSQUITTO MQ_SERVER\n");

    mosquitto_lib_init();

    (*mqtt) = mosquitto_new(NULL, true, NULL);
    if ((*mqtt) == NULL) {
        debug_error("Error: Out of memory.");
        return 1;
    }

    // mosquitto_connect_callback_set(params -> (*mqtt), on_connect);
    // mosquitto_subscribe_callback_set(params -> (*mqtt), on_subscribe);
    mosquitto_message_callback_set((*mqtt), on_message);
    mosquitto_int_option((*mqtt), mosq_opt_t::MOSQ_OPT_TCP_NODELAY, 1);

    int rc = mosquitto_connect((*mqtt), "localhost", 1883, 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy((*mqtt));
        debug_error("ERROR INIT MOSQUITTO MQ_SERVER: " << mosquitto_strerror(rc));
        return 1;
    }

    /* Run the network loop in a background thread, this call returns quickly. */
    rc = mosquitto_loop_start((*mqtt));
    if (rc != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy((*mqtt));
        debug_error("Error: " << mosquitto_strerror(rc));
        return 1;
    }

    // mosquitto_loop_forever(params -> mqtt, -1, 1);
    debug_info("END INIT MOSQUITTO MQ_SERVER\n");

    // Mosquitto pool thread
    int nthreads_mq = 128;
    pthread_t threads_mq[nthreads_mq];
    mq_server_utils::queue_mq_init();

    // Crear el grupo de hilos (thread pool)
    for (int i = 0; i < nthreads_mq; i++) {
        pthread_create(&threads_mq[i], NULL, process_message, &i);
    }

    return 0;  // OK: 0, ERROR: 1
}

int mq_server_comm::mq_server_mqtt_destroy(struct mosquitto *mqtt) {
    debug_info("BEGIN DESTROY MOSQUITTO MQ_SERVER\n");
    mosquitto_lib_cleanup();
    mosquitto_loop_stop(mqtt, true);
    debug_info("END DESTROY MOSQUITTO MQ_SERVER\n");
    return 0;  // OK: 0, ERROR: 1
}

/* ................................................................... */

}  // namespace XPN