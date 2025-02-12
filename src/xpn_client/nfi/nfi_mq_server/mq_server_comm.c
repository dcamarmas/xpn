/*
 *  Copyright 2020-2023 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos
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

     #include "mq_server_comm.h"


  /* ... Functions / Funciones ......................................... */

int tcpClient_comm_init ( __attribute__((__unused__)) tcpClient_param_st * params )
{
    // int ret ;

    debug_info("[NFI_MQ_COMM] begin tcpClient_comm_init(...)\n");

    debug_info("[NFI_MQ_COMM] end tcpClient_comm_init(...)\n");

    // Return OK
    return 0;
}


int tcpClient_comm_destroy ( __attribute__((__unused__)) tcpClient_param_st * params )
{
    // int ret ;

    debug_info("[NFI_MQ_COMM] begin tcpClient_comm_destroy(...)\n");

    debug_info("[NFI_MQ_COMM] end tcpClient_comm_destroy(...)\n");

    // Return OK
    return 0;
}

int doConnection ( tcpClient_param_st * params)
{
    if ( params -> xpn_keep_connection == 0 )
        return connection ( params );
    else 
        return 0;
}

int connection ( tcpClient_param_st * params )
{
    if ( params -> server != -1 )
    {
        doDisconnection ( params );
    }

    struct hostent * hp;
    struct sockaddr_in server_addr;
    int ret, sd, flag, val;
    int data;

    // Socket...
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        perror("socket: ");
        return -1;
    }
    // Set sockopt
    flag = 1;
    ret = setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, & flag, sizeof(flag)) ;
    if (ret < 0) {
        perror("setsockopt");
        return -1;
    }

    val = 1024 * 1024; //1 MB
    ret = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char * ) & val, sizeof(int)) ;
    if (ret < 0) {
        perror("setsockopt");
        return -1;
    }

    val = 1024 * 1024; //1 MB
    ret = setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char * ) & val, sizeof(int)) ;
    if (ret < 0) {
        perror("setsockopt");
        return -1;
    }

    // gethost by name
    hp = gethostbyname(params -> server_name);
    if (hp == NULL)
    {
        debug_info(stderr, "nfi_mq_server_init: error gethostbyname %s (%s,%s)\n",
            params -> srv_name, params -> server_name, params->port_number);
        return -1;
    }

    // Connect...
    bzero((char * ) & server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(atoi(params->port_number));
    memcpy( & (server_addr.sin_addr), hp -> h_addr, hp -> h_length );

    ret = connect(sd, (struct sockaddr * ) & server_addr, sizeof(server_addr));

    if (ret < 0)
    {
        debug_info(stderr, "nfi_mq_server_init: error in connect %s (%s,%s)\n",
                    params -> srv_name, params -> server_name, params->port_number);
        return -1;
    }

    params->server = sd;

    // send ok message
    data = 0;
    ret = tcpClient_write_data(params -> server, (char *)&data, 1 * sizeof(int), "<unused msg_id>") ;
    if (ret < 0) 
    {
        debug_warning("Server[?]: doConnection fails");
        return -1;
    }

    return ret;
}


int doDisconnection ( tcpClient_param_st * params )
{
    int ret;
    
    if ( params -> xpn_keep_connection == 1 )
        return 0;

    if ( params -> server == -1 )
    {
        debug_info("Server: doDisconnection fails\n");
        return -1;
    }

    ret = close ( params -> server );
    if (ret < 0) 
    {
        debug_warning("Server[?]: doDisconnection fails");
        return -1;
    }

    params -> server = -1;

    return ret;
}



int tcpClient_comm_connect ( tcpClient_param_st * params )
{
    struct hostent * hp;
    struct sockaddr_in server_addr;
    int ret, sd, flag, val;
    int lookup_retries;
    int data;

    debug_info("[NFI_MQ_COMM] begin tcpClient_comm_connect(...)\n");

    // Lookup port name
    lookup_retries = 0;
    do
    {
        debug_info("[%s][%d]\t1-%s 2-%s 3-%s\n", __FILE__, __LINE__, params -> srv_name, params -> server_name, params -> port_number);

        ret = ns_lookup("mq_server", params -> srv_name, params -> server_name, params -> port_number) ;

        debug_info("[%s][%d]\t1-%s 2-%s 3-%s 4-%d\n", __FILE__, __LINE__, params -> srv_name, params -> server_name, params -> port_number, ret);
        if (ret < 0)
        {
            if (lookup_retries == 0)
            {
                char cli_name[HOST_NAME_MAX];
                gethostname(cli_name, HOST_NAME_MAX);
                debug_info("----------------------------------------------------------------\n");
                debug_info("XPN Client %s : Waiting for servers being up and running...\n", cli_name);
                debug_info("----------------------------------------------------------------\n\n");
            }
            lookup_retries++;
            sleep(2);
        }
    } while ((ret < 0) && (lookup_retries < 150));

    if (ret < 0) {
        debug_error("ERROR: DNS Lookup %s Port %s\n", params -> server_name, params -> port);
        return -1;
    }

    debug_info("[NFI_MQ_COMM] ----SERVER = %s NEWSERVER = %s PORT = %s\n", params -> srv_name, params -> server_name, params->port_number);


    ret = connection( params );
    if (ret < 0) 
    {
        debug_info(stderr, "ERROR: tcpClient_comm_connect: Connection failed\n");
        return -1;
    }
    

    return ret;
}


int tcpClient_comm_disconnect ( __attribute__((__unused__)) tcpClient_param_st * params )
{
    debug_info("[NFI_MQ_COMM] begin tcpClient_comm_disconnect nservers\n");

    debug_info("[NFI_MQ_COMM] end   tcpClient_comm_disconnect nservers\n");

    // Return OK
    return 0;
}


int tcpClient_comm_locality ( tcpClient_param_st * params )
{
    int ret;
    int data;
    char cli_name[HOST_NAME_MAX];
    char serv_name[HOST_NAME_MAX];

    debug_info("[NFI_MQ_COMM] begin tcpClient_comm_locality\n");

    // Locality disable
    if (!params -> xpn_locality)
    {
        debug_info("[NFI_MQ_COMM] tcpClient_comm_locality disable\n");
        params -> locality = 0;
        return 1;
    }

    // Get client host name (send id, recv strlen, recv string)
    gethostname(cli_name, HOST_NAME_MAX);

    data = MQ_SERVER_GETNODENAME;
    ret = tcpClient_write_data(params -> server, (char *)&data, 1 * sizeof(int), "<unused msg_id>") ;
    if (ret < 0)
    {
        debug_info("Server[?]: TCP_Send fails :-(");
        return -1;
    }

    ret = tcpClient_read_data( params -> server, serv_name, HOST_NAME_MAX * sizeof(char), "<unused msg_id>") ;
    if (ret < 0)
    {
        debug_info("Server[?]: tcpClient_read_data fails :-(");
        return -1;
    }

    /*
    ret = tcpClient_read_data( params -> server, params -> sem_name_server, PATH_MAX * sizeof(char), "<unused msg_id>") ;
    if (ret < 0)
    {
        debug_info("Server[?]: tcpClient_read_data fails :-(");
        return -1;
    }
    */

    // check locality
    params -> locality = 0;
    if (strcmp(cli_name, serv_name) == 0)
    {
        params -> locality = 1;
        // params -> sem_server = sem_open(params -> sem_name_server, 0);
    }

    debug_info("[NFI_MQ_COMM] end tcpClient_comm_locality\n");

    // Return OK
    return 1;
}


ssize_t tcpClient_write_operation ( int fd, char * data, ssize_t size, __attribute__((__unused__)) char * msg_id )
{
    int ret;

    debug_info("[NFI_MQ_COMM] begin tcpClient_write_operation(...)\n");

    // Check params
    if (size == 0) {
        debug_info("Server[?]: size == 0");
        return 0;
    }
    if (size < 0) {
        debug_warning("Server[?]: size < 0");
        return -1;
    }

    ret = tcpClient_write_data(fd, data, size * sizeof(int), msg_id) ;

    debug_info("[NFI_MQ_COMM] end tcpClient_write_operation(...)\n");

    // Return integers written
    return ret / sizeof(int);
}


ssize_t tcpClient_write_data ( int fd, char * data, ssize_t size, __attribute__((__unused__)) char * msg_id )
{
    int ret, cont;
    static ssize_t( * real_write)(int,const void * , size_t) = NULL;

    debug_info("[NFI_MQ_COMM] begin tcpClient_write_data(...)\n");

    // Check params
    if (size == 0) {
        return 0;
    }
    if (size < 0) {
        debug_warning("Server[?]: size < 0");
        return -1;
    }

    if (NULL == real_write) 
    {
        real_write = (ssize_t( * )(int,const void * , size_t)) dlsym(RTLD_NEXT, "write");
    }

    cont = 0 ;
    do
    {
        ret = real_write(fd, data + cont, size - cont);

        debug_info("[NFI_MQ_COMM] client: write_data(%d): %lu = %d ID=%s --th:%d--\n", fd, (unsigned long) size, ret, msg_id, (int) pthread_self());

        if (ret < 0) {
            //debug_info("[NFI_MQ_COMM] client: write_data(%d): %lu = %d ID=%s --th:%d--\n", fd, (unsigned long) size, ret, msg_id, (int) pthread_self());

            perror("tcpClient_write_data: ERROR on real_write: ");
            return ret ;
	   }

        cont += ret;

    } while ((ret > 0) && (cont != size));

    if (ret < 0) {
        debug_info(stderr, "[NFI_MQ_COMM]  ERROR: write_data(%d): err %d  ID=%s --th:%d--\n", fd, ret, msg_id, (int) pthread_self());
        return ret;
    }

    debug_info("[NFI_MQ_COMM] client: write_data(%d): %d de %lu ID=%s --th:%d--\n", fd, cont, (unsigned long) size, msg_id, (int) pthread_self());
    debug_info("[NFI_MQ_COMM] end tcpClient_write_data(...)\n");

    debug_info("-------------SIZE = %d\n", size);

    // Return bytes written
    return cont;
}


ssize_t tcpClient_read_data ( int fd, char * data, ssize_t size, __attribute__((__unused__)) char * msg_id )
{
    int ret, cont;
    static ssize_t (* real_read)(int, void * , size_t) = NULL;

    debug_info("[NFI_MQ_COMM] begin tcpClient_read_data(...)\n");

    // Check params
    if (size == 0) {
        return 0;
    }
    if (size < 0) {
        debug_warning("Server[?]: size < 0");
        return -1;
    }

    debug_info("tcpClient_read_data %d\n", fd);

    if (NULL == real_read) 
    {
        real_read = (ssize_t( * )(int, void * , size_t)) dlsym(RTLD_NEXT, "read");
    }

    cont = 0;
    do
    {
        ret = real_read(fd, data + cont, size - cont);

        debug_info("[NFI_MQ_COMM] client: read_data(%d): %lu = %d ID=%s --th:%d--\n", fd, (unsigned long) size, ret, msg_id, (int) pthread_self());

        if (ret < 0) {
            perror("tcpClient_read_data: ERROR on real_read: ");
	    return ret ;
        }

        cont += ret;

    } while ((ret > 0) && (cont != size));

    if (ret < 0) {
        debug_info(stderr, "[NFI_MQ_COMM]  client: read_data(%d): err %d  ID=%s --th:%d--\n", fd, ret, msg_id, (int) pthread_self());
        return ret;
    }

    debug_info("[NFI_MQ_COMM] client: read_data(%d): %d de %lu ID=%s --th:%d--\n", fd, cont, (unsigned long) size, msg_id, (int) pthread_self());
    debug_info("[NFI_MQ_COMM] end tcpClient_read_data(...)\n");

    // Return bytes read
    return cont;
}


  /* ................................................................... */

