/* FILE: ckpttimer.cpp
 * AUTHOR: Rohan Garg
 * EMAIL: rohgarg@ccs.neu.edu
 * Copyright (C) 2015 Rohan Garg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "dmtcp.h"
#include "xpn.h"

static void xpn_event_hook(DmtcpEvent_t event, [[maybe_unused]] DmtcpEventData_t *data) {
    int res = 0;

    switch (event) {
        case DMTCP_EVENT_INIT:
            printf("DMTCP_EVENT_INIT\n");
            break;

        case DMTCP_EVENT_EXIT:
            printf("DMTCP_EVENT_EXIT\n");
            break;

        case DMTCP_EVENT_PRESUSPEND:
            printf("DMTCP_EVENT_PRESUSPEND\n");
            // xpn_clean_connections();
            // printf("DMTCP_EVENT_PRESUSPEND end = %d\n", res);
            break;
        case DMTCP_EVENT_PRECHECKPOINT:
            printf("DMTCP_EVENT_PRECHECKPOINT\n");
            xpn_clean_connections();
            
            printf("DMTCP_EVENT_PRECHECKPOINT end = %d\n", res);
            break;

        case DMTCP_EVENT_RESUME:
            printf("DMTCP_EVENT_RESUME\n");
            res = xpn_init();
            printf("DMTCP_EVENT_PRESUSPEND end = %d\n", res);
            break;

        case DMTCP_EVENT_RESTART:
            printf("DMTCP_EVENT_RESTART\n");
            res = xpn_init();
            printf("DMTCP_EVENT_RESTART end = %d\n", res);
            break;

        case DMTCP_EVENT_ATFORK_CHILD:
            printf("DMTCP_EVENT_ATFORK_CHILD\n");
            break;

        default:
            break;
    }
    fflush(stdin);
}

DmtcpPluginDescriptor_t xpn_plugin = {DMTCP_PLUGIN_API_VERSION, PACKAGE_VERSION, "xpn",         "xpn",
                                      "xpn@gmail.todo",         "XPN plugin",    xpn_event_hook};

DMTCP_DECL_PLUGIN(xpn_plugin);
