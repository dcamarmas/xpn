
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

#include <pthread.h>
#include <sys/time.h>

/* ... Const / Constantes ............................................ */

namespace XPN {
#define QUEUE_MQ_SIZE 1000000

/* ... Data Types / Tipo de datos .................................... */

struct ThreadData {
    char* topic;
    char* msg;
};

struct CircularQueueMQ {
    ThreadData* queue[QUEUE_MQ_SIZE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

/* ... Functions / Funciones ......................................... */
class mqtt_server_utils {
   public:
    static CircularQueueMQ queue_mq;

    static double get_time(void);
    static double get_time_ops(void);

    static void queue_mq_init(void);
    static void enqueue_mq(ThreadData* client);
    static ThreadData* dequeue_mq(void);
};

/* ................................................................... */

}  // namespace XPN