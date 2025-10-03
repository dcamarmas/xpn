/*
 *  Copyright 2020-2025 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
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

   #include "xpn_server_ops.h"
   #include "xpn_server_params.h"
   #include "xpn_server_comm.h"


/* ... Functions / Funciones ......................................... */

    // File operations
    void xpn_server_op_open        ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_creat       ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_read        ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_write       ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_close       ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_rm          ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_rm_async    ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_rename      ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_setattr     ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_getattr     ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;

    // Directory operations
    void xpn_server_op_mkdir       ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_opendir     ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_readdir     ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_closedir    ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_rmdir       ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_rmdir_async ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;

    // FS Operations
    void xpn_server_op_getnodename ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id );
    void xpn_server_op_fstat       ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ); // TODO

    // Metadata
    void xpn_server_op_read_mdata            ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_write_mdata           ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;
    void xpn_server_op_write_mdata_file_size ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id ) ;


    //Read the operation to realize
    int xpn_server_do_operation ( int server_type, struct st_th * th, int * the_end )
    {
        int ret;
        struct st_xpn_server_msg head;

        debug_info("[TH_ID=%d] [XPN_SERVER_OPS] [xpn_server_do_operation] >> Begin\n", th->id);
        debug_info("[TH_ID=%d] [XPN_SERVER_OPS] [xpn_server_do_operation] OP '%s'; OP_ID %d\n", th->id, xpn_server_op2string(th->type_op), th->type_op);

        switch (th->type_op)
	{
            //File API
        case XPN_SERVER_OPEN_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_open), sizeof(head.u_st_xpn_server_msg.op_open), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_open(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_CREAT_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_creat), sizeof(head.u_st_xpn_server_msg.op_creat), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_creat(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_READ_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_read), sizeof(head.u_st_xpn_server_msg.op_read), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_read(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_WRITE_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_write), sizeof(head.u_st_xpn_server_msg.op_write), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_write(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_CLOSE_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_close), sizeof(head.u_st_xpn_server_msg.op_close), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_close(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_RM_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_rm), sizeof(head.u_st_xpn_server_msg.op_rm), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_rm(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_RM_FILE_ASYNC:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_rm), sizeof(head.u_st_xpn_server_msg.op_rm), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_rm_async(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_RENAME_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_rename), sizeof(head.u_st_xpn_server_msg.op_rename), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_rename(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_GETATTR_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_getattr), sizeof(head.u_st_xpn_server_msg.op_getattr), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_getattr(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_SETATTR_FILE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_setattr), sizeof(head.u_st_xpn_server_msg.op_setattr), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_setattr(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;

            //Directory API
        case XPN_SERVER_MKDIR_DIR:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_mkdir), sizeof(head.u_st_xpn_server_msg.op_mkdir), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_mkdir(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_OPENDIR_DIR:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_opendir), sizeof(head.u_st_xpn_server_msg.op_opendir), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_opendir(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_READDIR_DIR:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_readdir), sizeof(head.u_st_xpn_server_msg.op_readdir), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_readdir(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_CLOSEDIR_DIR:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_closedir), sizeof(head.u_st_xpn_server_msg.op_closedir), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_closedir(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_RMDIR_DIR:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_rmdir), sizeof(head.u_st_xpn_server_msg.op_rmdir), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_rmdir(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_RMDIR_DIR_ASYNC:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_rmdir), sizeof(head.u_st_xpn_server_msg.op_rmdir), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_rmdir_async(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_READ_MDATA:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_read_mdata), sizeof(head.u_st_xpn_server_msg.op_read_mdata), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_read_mdata(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_WRITE_MDATA:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_write_mdata), sizeof(head.u_st_xpn_server_msg.op_write_mdata), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_write_mdata(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;
        case XPN_SERVER_WRITE_MDATA_FILE_SIZE:
             ret = xpn_server_comm_read_data(server_type, th->comm, (char * ) & (head.u_st_xpn_server_msg.op_write_mdata_file_size), sizeof(head.u_st_xpn_server_msg.op_write_mdata_file_size), th->rank_client_id, th->tag_client_id);
             if (ret != -1) {
                 xpn_server_op_write_mdata_file_size(th->params, th->comm, & head, th->rank_client_id, th->tag_client_id);
             }
             break;

            //Connection API
        case XPN_SERVER_DISCONNECT:
             break;

        case XPN_SERVER_FINALIZE:
             *the_end = 1;
             break;
        }

        debug_info("[TH_ID=%d] [XPN_SERVER_OPS] [xpn_server_do_operation] << End\n", th->id);
        return 0;
    }

    int xpn_server_read_path ( int server_type, void *comm, char *full_path, int full_path_size, char *path_msg, int path_len, int rank_client_id, int tag_client_id )
    {
        ssize_t r ;

        bzero (full_path, full_path_size);
        memcpy(full_path, path_msg, path_len > XPN_PATH_MAX ? XPN_PATH_MAX : path_len);

        if (path_len > XPN_PATH_MAX)
        {
            r = xpn_server_comm_read_data(server_type, comm, full_path + XPN_PATH_MAX, path_len - XPN_PATH_MAX, rank_client_id, tag_client_id);
            if (r < 0) {
                full_path[0] = '\0';
            }
        }
        full_path[path_len] = '\0';

        return 0;
    }

    // File API
    void xpn_server_op_open ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_open] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_open.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_open.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_open] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_open] open(%s, %d, %d)\n", params->rank, full_path, head->u_st_xpn_server_msg.op_open.flags, head->u_st_xpn_server_msg.op_open.mode);

        // do open
        status.ret = filesystem_open2(full_path, head->u_st_xpn_server_msg.op_open.flags, head->u_st_xpn_server_msg.op_open.mode);
        status.server_errno = errno;
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_open] open(%s)=%d\n", params->rank, full_path, status.ret);
        if (status.ret < 0) {
            xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);
            debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_open] << End\n", params->rank);
	    return ;
        }

        if (head->u_st_xpn_server_msg.op_open.xpn_session == 0) {
            status.ret = filesystem_close(status.ret);
        }
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        // If this is a file with mq_server protocol then the server is going to suscribe
        if (head->u_st_xpn_server_msg.op_open.file_type == 1) {
            mq_server_op_subscribe(params, head);
        }

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_open] << End\n", params->rank);
    }

    void xpn_server_op_creat ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_creat] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_creat.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_creat.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_creat] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_creat] creat(%s)\n", params->rank, full_path);

        // do creat
        status.ret = filesystem_creat(full_path, head->u_st_xpn_server_msg.op_creat.mode);
        status.server_errno = errno;
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_creat] creat(%s)=%d\n", params->rank, full_path, status.ret);
        if (status.ret < 0) {
            xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);
	    return ;
        }

        status.ret = filesystem_close(status.ret);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        // If this is a file with mq_server protocol then the server is going to suscribe
        if (head->u_st_xpn_server_msg.op_creat.file_type == 1) {
            mq_server_op_subscribe(params, head);
        }

        // show debug info
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_creat] << End\n", params->rank);
    }

    void xpn_server_op_read ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_rw_req req;
        char * buffer = NULL;
        long size, diff, to_read, cont;
        off_t ret_lseek;
        int fd;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_read.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_read.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] read(%s, %ld %ld)\n", params->rank, full_path, head->u_st_xpn_server_msg.op_read.offset, head->u_st_xpn_server_msg.op_read.size);

        // initialize counters
        cont = 0;
        size = head->u_st_xpn_server_msg.op_read.size;
        if (size > MAX_BUFFER_SIZE) {
            size = MAX_BUFFER_SIZE;
        }
        diff = head->u_st_xpn_server_msg.op_read.size - cont;

        //Open file
        if (head->u_st_xpn_server_msg.op_read.xpn_session == 1)
             fd = head->u_st_xpn_server_msg.op_read.fd;
        else fd = filesystem_open(full_path, O_RDONLY);
        if (fd < 0) {
            req.size = -1;
            req.status.ret = fd;
            req.status.server_errno = errno;
            xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_rw_req), rank_client_id, tag_client_id);
            goto cleanup_xpn_server_op_read;
        }

        // malloc a buffer of size...
        buffer = (char * ) malloc(size);
        if (NULL == buffer) {
            req.size = -1;
            req.status.ret = -1;
            req.status.server_errno = errno;
            xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_rw_req), rank_client_id, tag_client_id);
            goto cleanup_xpn_server_op_read;
        }

        // loop...
        do
	{
            if (diff > size)
                to_read = size;
            else to_read = diff;

            // lseek and read data...
            ret_lseek = filesystem_lseek(fd, head->u_st_xpn_server_msg.op_read.offset + cont, SEEK_SET);
            if (ret_lseek == -1) {
                req.size = -1;
                req.status.ret = -1;
                req.status.server_errno = errno;
                xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_rw_req), rank_client_id, tag_client_id);
                goto cleanup_xpn_server_op_read;
            }

            req.size = filesystem_read(fd, buffer, to_read);
            // if error then send as "how many bytes" -1
            if (req.size < 0 || req.status.ret == -1) {
                req.size = -1;
                req.status.ret = -1;
                req.status.server_errno = errno;
                xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_rw_req), rank_client_id, tag_client_id);
                goto cleanup_xpn_server_op_read;
            }
            // send (how many + data) to client...
            req.status.ret = 0;
            req.status.server_errno = errno;
            xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_rw_req), rank_client_id, tag_client_id);
            debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] op_read: send size %ld\n", params->rank, req.size);

            // send data to client...
            if (req.size > 0) {
                xpn_server_comm_write_data(params->server_type, comm, buffer, req.size, rank_client_id, tag_client_id);
                debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] op_read: send data\n", params->rank);
            }
            cont = cont + req.size; //Send bytes
            diff = head->u_st_xpn_server_msg.op_read.size - cont;

        } while ((diff > 0) && (req.size != 0));

cleanup_xpn_server_op_read:
        if (head->u_st_xpn_server_msg.op_read.xpn_session == 0) {
           filesystem_close(fd);
        }

        // free buffer
        FREE_AND_NULL(buffer);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] read(%s, %ld %ld)=%ld\n", params->rank, full_path, head->u_st_xpn_server_msg.op_read.offset, head->u_st_xpn_server_msg.op_read.size, cont);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read] << End\n", params->rank);
    }

    void xpn_server_op_write ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_rw_req req;
        char * buffer = NULL;
        int size, diff, cont, to_write;
        off_t ret_lseek;
        int fd, ret;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write] ERROR: NULL arguments\n", -1);
            return;
        }

        // If this is a file with mq_server protocol then the callback function is going to be used
        if (head->u_st_xpn_server_msg.op_write.file_type == 1) {
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_write.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_write.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write] write(%s, %ld %ld)\n", params->rank, full_path, head->u_st_xpn_server_msg.op_write.offset, head->u_st_xpn_server_msg.op_write.size);

        // initialize counters
        cont = 0;
        size = (head->u_st_xpn_server_msg.op_write.size);
        if (size > MAX_BUFFER_SIZE) {
            size = MAX_BUFFER_SIZE;
        }
        diff = head->u_st_xpn_server_msg.op_write.size - cont;

        //Open file
        if (head->u_st_xpn_server_msg.op_write.xpn_session == 1)
             fd = head->u_st_xpn_server_msg.op_write.fd;
        else fd = filesystem_open(full_path, O_WRONLY);
        if (fd < 0) {
            req.size = -1;
            req.status.ret = -1;
            goto cleanup_xpn_server_op_write;
        }

        // malloc a buffer of size...
        buffer = (char * ) malloc(size);
        if (NULL == buffer) {
            req.size = -1;
            req.status.ret = -1;
            goto cleanup_xpn_server_op_write;
        }

        // loop...
        do
	{
            if (diff > size)
                 to_write = size;
            else to_write = diff;

            // read data from MPI and write into the file
            ret = xpn_server_comm_read_data(params->server_type, comm, buffer, to_write, rank_client_id, tag_client_id);
            if (ret < 0) {
                req.status.ret = -1;
                goto cleanup_xpn_server_op_write;
            }

            ret_lseek = filesystem_lseek(fd, head->u_st_xpn_server_msg.op_write.offset + cont, SEEK_SET);
            if (ret_lseek < 0) {
                req.status.ret = -1;
                goto cleanup_xpn_server_op_write;
            }
            req.size = filesystem_write(fd, buffer, to_write);
            
            if (req.size < 0) {
                req.status.ret = -1;
                goto cleanup_xpn_server_op_write;
            }

            // update counters
            cont = cont + req.size; // Received bytes
            diff = head->u_st_xpn_server_msg.op_write.size - cont;
        } while ((diff > 0) && (req.size != 0));

        req.size = cont;
        req.status.ret = 0;

cleanup_xpn_server_op_write:
        // write to the client the status of the write operation
        req.status.server_errno = errno;

        xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_rw_req), rank_client_id, tag_client_id);

        if (head->u_st_xpn_server_msg.op_write.xpn_session == 1)
             filesystem_fsync(fd);
        else filesystem_close(fd);

        // free buffer
        FREE_AND_NULL(buffer);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write] write(%s, %ld %ld)=%d\n", params->rank, full_path, head->u_st_xpn_server_msg.op_write.offset, head->u_st_xpn_server_msg.op_write.size, cont);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write] << End\n", params->rank);
    }

    void xpn_server_op_close ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_close] ERROR: NULL arguments\n", -1);
            return;
        }

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_close] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_close] close(%d)\n", params->rank, head->u_st_xpn_server_msg.op_close.fd);

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_close.path_len;
	char *path_msg = head->u_st_xpn_server_msg.op_close.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        // do rm
        status.ret = filesystem_close(head->u_st_xpn_server_msg.op_close.fd);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        // Si es un fichero con protocolo mq_server se desuscribe
        if (head->u_st_xpn_server_msg.op_close.file_type == 1) {
            mq_server_op_unsubscribe(params, head);
        }

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_close] close(%d)=%d\n", params->rank, head->u_st_xpn_server_msg.op_close.fd, status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_close] << End\n", params->rank);
    }

    void xpn_server_op_rm ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_rm.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_rm.path ;

        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm] unlink(%s)\n", params->rank, full_path);

        // do rm
        status.ret = filesystem_unlink(full_path);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm] unlink(%s)=%d\n", params->rank, full_path, status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm] << End\n", params->rank);
    }

    void xpn_server_op_rm_async ( xpn_server_param_st * params, __attribute__((__unused__)) void * comm, struct st_xpn_server_msg * head, __attribute__((__unused__)) int rank_client_id, __attribute__((__unused__)) int tag_client_id )
    {
        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm_async] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_rm.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_rm.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm_async] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm_async] unlink(%s)\n", params->rank, head->u_st_xpn_server_msg.op_rm.path);

        // do rm
        filesystem_unlink(full_path);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm_async] unlink(%s)=%d\n", params->rank, head->u_st_xpn_server_msg.op_rm.path, 0);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rm_async] << End\n", params->rank);
    }

    void xpn_server_op_rename(xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id)
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rename] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path_old[PATH_MAX];
        int   path_len_old = head->u_st_xpn_server_msg.op_rename.old_url_len ;
        char *path_msg_old = head->u_st_xpn_server_msg.op_rename.old_url ;
        xpn_server_read_path(params->server_type, comm, full_path_old, PATH_MAX, path_msg_old, path_len_old, rank_client_id, tag_client_id) ;

        char  full_path_new[PATH_MAX];
        int   path_len_new = head->u_st_xpn_server_msg.op_rename.new_url_len;
        char *path_msg_new = head->u_st_xpn_server_msg.op_rename.new_url ;
        xpn_server_read_path(params->server_type, comm, full_path_new, PATH_MAX, path_msg_new, path_len_new, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rename] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rename] rename(%s, %s)\n", params->rank, full_path_old, full_path_new);

        // do rename
        status.ret = filesystem_rename(full_path_old, full_path_new);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rename] rename(%s, %s)=%d\n", params->rank, full_path_old, full_path_new, status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rename] << End\n", params->rank);
    }

    void xpn_server_op_getattr ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_attr_req req;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_getattr] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_getattr.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_getattr.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_getattr] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_getattr] stat(%s)\n", params->rank, full_path);

        // do getattr
        req.status = filesystem_stat(full_path, &(req.attr)) ;
        req.status_req.ret          = req.status;
        req.status_req.server_errno = errno;

        xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_attr_req), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_getattr] stat(%s)=%d\n", params->rank, head->u_st_xpn_server_msg.op_getattr.path, req.status);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_getattr] << End\n", params->rank);
    }

    void xpn_server_op_setattr ( xpn_server_param_st * params, __attribute__((__unused__)) void * comm, struct st_xpn_server_msg * head, __attribute__((__unused__)) int rank_client_id, __attribute__((__unused__)) int tag_client_id )
    {
        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_setattr] ERROR: NULL arguments\n", -1);
            return;
        }

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_setattr] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_setattr] SETATTR(...)\n", params->rank);

        if (NULL == head) {
            return;
        }

        // do setattr
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_setattr] SETATTR operation to be implemented !!\n", params->rank);
	// TODO: implementation of setattr !!

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_setattr] SETATTR(...)=(...)\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_setattr] << End\n", params->rank);
    }

    //Directory API
    void xpn_server_op_mkdir ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_mkdir] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_mkdir.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_mkdir.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_mkdir] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_mkdir] mkdir(%s)\n", params->rank, full_path);

        // do mkdir
        status.ret = filesystem_mkdir(full_path, head->u_st_xpn_server_msg.op_mkdir.mode);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_mkdir] mkdir(%s)=%d\n", params->rank, full_path, status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_mkdir] << End\n", params->rank);
    }

    void xpn_server_op_opendir ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        DIR    * ret;
        struct st_xpn_server_opendir_req req;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_opendir] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_opendir.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_opendir.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_opendir] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_opendir] opendir(%s)\n", params->rank, full_path);


        ret = filesystem_opendir(full_path);
        req.status.ret = ret == NULL ? -1 : 0;
        req.status.server_errno = errno;

        if (req.status.ret == 0)
	{
            if (head->u_st_xpn_server_msg.op_opendir.xpn_session == 1)
	    {
                req.dir = ret;
            }
	    else
	    {
                req.status.ret = filesystem_telldir(ret);
            }
            req.status.server_errno = errno;
        }

        if (head->u_st_xpn_server_msg.op_opendir.xpn_session == 0) {
            filesystem_closedir(ret);
        }

        xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_opendir_req), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_opendir] opendir(%s)=%p\n", params->rank, full_path, ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_opendir] << End\n", params->rank);
    }

    void xpn_server_op_readdir ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct dirent * ret;
        struct st_xpn_server_readdir_req ret_entry;
        DIR  * s = NULL;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_readdir] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_readdir.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_readdir.path ;

        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_readdir] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_readdir] readdir(%s)\n", params->rank, full_path);


        if (head->u_st_xpn_server_msg.op_readdir.xpn_session == 1)
	{
            // Reset errno
            errno = 0;
            ret = filesystem_readdir(head->u_st_xpn_server_msg.op_readdir.dir);
        }
	else
	{
            s = filesystem_opendir(full_path);
            ret_entry.status.ret = s == NULL ? -1 : 0;
            ret_entry.status.server_errno = errno;

            filesystem_seekdir(s, head->u_st_xpn_server_msg.op_readdir.telldir);

            // Reset errno
            errno = 0;
            ret = filesystem_readdir(s);
        }

        if (ret != NULL)
	{
            ret_entry.end = 1;
            ret_entry.ret = * ret;
        }
	else
	{
            ret_entry.end = 0;
        }

        ret_entry.status.ret = ret == NULL ? -1 : 0;

        if (head->u_st_xpn_server_msg.op_readdir.xpn_session == 0) {
            ret_entry.telldir    = filesystem_telldir(s);
            ret_entry.status.ret = filesystem_closedir(s);
        }
        ret_entry.status.server_errno = errno;

        xpn_server_comm_write_data(params->server_type, comm, (char * ) & ret_entry, sizeof(struct st_xpn_server_readdir_req), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_readdir] readdir(%p)=%p\n", params->rank, s, ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_readdir] << End\n", params->rank);
    }

    void xpn_server_op_closedir ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_closedir] ERROR: NULL arguments\n", -1);
            return;
        }

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_closedir] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_closedir] closedir(%ld)\n", params->rank, head->u_st_xpn_server_msg.op_closedir.dir);

        // do rm
        status.ret = filesystem_closedir(head->u_st_xpn_server_msg.op_closedir.dir);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_closedir] closedir(%d)=%d\n", params->rank, head->u_st_xpn_server_msg.op_closedir.dir, status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_closedir] << End\n", params->rank);
    }

    void xpn_server_op_rmdir ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        struct st_xpn_server_status status;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_rmdir.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_rmdir.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir] rmdir(%s)\n", params->rank, full_path);

        // do rmdir
        status.ret = filesystem_rmdir(full_path);
        status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & status, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir] rmdir(%s)=%d\n", params->rank, full_path, status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir] << End\n", params->rank);
    }

    void xpn_server_op_rmdir_async ( __attribute__((__unused__)) xpn_server_param_st * params, __attribute__((__unused__)) void * comm, struct st_xpn_server_msg * head, __attribute__((__unused__)) int rank_client_id, __attribute__((__unused__)) int tag_client_id )
    {
        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir_async] ERROR: NULL arguments\n", -1);
            return;
        }

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir_async] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir_async] rmdir(%s)\n", params->rank, head->u_st_xpn_server_msg.op_rmdir.path);

        // do rmdir
        filesystem_rmdir(head->u_st_xpn_server_msg.op_rmdir.path);
	// TODO: full_path needed ???

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir_async] rmdir(%s)=%d\n", params->rank, head->u_st_xpn_server_msg.op_rmdir.path, 0);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_rmdir_async] << End\n", params->rank);
    }

    void xpn_server_op_read_mdata ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        int ret, fd;
        struct st_xpn_server_read_mdata_req req = { 0 };

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read_mdata] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_read_mdata.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_read_mdata.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read_mdata] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read_mdata] read_mdata(%s)\n", params->rank, full_path);

        fd = filesystem_open(full_path, O_RDWR);
        if (fd < 0)
	{
            if (errno == EISDIR) {
                // if is directory there are no metadata to read so return 0
                ret = 0;
                memset( & req.mdata, 0, sizeof(struct xpn_metadata));
                goto cleanup_xpn_server_op_read_mdata;
            }

            ret = fd;
            goto cleanup_xpn_server_op_read_mdata;
        }

        ret = filesystem_read(fd, & req.mdata, sizeof(struct xpn_metadata));

        if (!XPN_CHECK_MAGIC_NUMBER( & req.mdata)) {
            memset( & req.mdata, 0, sizeof(struct xpn_metadata));
        }

        filesystem_close(fd); // TODO: think if necesary check error in close

cleanup_xpn_server_op_read_mdata:
        req.status.ret = ret;
        req.status.server_errno = errno;
        xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_read_mdata_req), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read_mdata] read_mdata(%s)=%d\n", params->rank, head->u_st_xpn_server_msg.op_read_mdata.path, req.status.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_read_mdata] << End\n", params->rank);
    }

    void xpn_server_op_write_mdata ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        int ret, fd;
        struct st_xpn_server_status req;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_write_mdata.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_write_mdata.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata] write_mdata(%s)\n", params->rank, full_path);

        

        fd = filesystem_open2(full_path, O_WRONLY | O_CREAT, S_IRWXU);
        if (fd < 0)
	{
            if (errno == EISDIR) {
                // if is directory there are no metadata to write so return 0
                ret = 0;
                goto cleanup_xpn_server_op_write_mdata;
            }

            ret = fd;
            goto cleanup_xpn_server_op_write_mdata;
        }

        ret = filesystem_write(fd, & head->u_st_xpn_server_msg.op_write_mdata.mdata, sizeof(struct xpn_metadata));

        filesystem_close(fd); //TODO: think if necesary check error in close

cleanup_xpn_server_op_write_mdata:
        req.ret = ret;
        req.server_errno = errno;

        xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata] write_mdata(%s)=%d\n", params->rank, head->u_st_xpn_server_msg.op_write_mdata.path, req.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata] << End\n", params->rank);
    }


    pthread_mutex_t op_write_mdata_file_size_mutex = PTHREAD_MUTEX_INITIALIZER;

    void xpn_server_op_write_mdata_file_size ( xpn_server_param_st * params, void * comm, struct st_xpn_server_msg * head, int rank_client_id, int tag_client_id )
    {
        int ret, fd;
        ssize_t actual_file_size = 0;
        struct st_xpn_server_status req;

        // check params...
        if (NULL == params) {
            printf("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] ERROR: NULL arguments\n", -1);
            return;
        }

        char  full_path[PATH_MAX];
        int   path_len = head->u_st_xpn_server_msg.op_write_mdata_file_size.path_len;
        char *path_msg = head->u_st_xpn_server_msg.op_write_mdata_file_size.path ;
        xpn_server_read_path(params->server_type, comm, full_path, PATH_MAX, path_msg, path_len, rank_client_id, tag_client_id) ;

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] >> Begin\n", params->rank);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] write_mdata_file_size(%s, %ld)\n", params->rank, full_path, head->u_st_xpn_server_msg.op_write_mdata_file_size.size);

        

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] mutex lock\n", params->rank);
        pthread_mutex_lock( & op_write_mdata_file_size_mutex);

        fd = filesystem_open(full_path, O_RDWR);
        if (fd < 0)
	{
            if (errno == EISDIR)
	    {
                // if is directory there are no metadata to write so return 0
                ret = 0;
                goto cleanup_xpn_server_op_write_mdata_file_size;
            }

            ret = fd;
            goto cleanup_xpn_server_op_write_mdata_file_size;
        }

        filesystem_lseek(fd, offsetof(struct xpn_metadata, file_size), SEEK_SET);
        ret = filesystem_read(fd, & actual_file_size, sizeof(ssize_t));
        if ((ret > 0) && (actual_file_size < head->u_st_xpn_server_msg.op_write_mdata_file_size.size)) {
            filesystem_lseek(fd, offsetof(struct xpn_metadata, file_size), SEEK_SET);
            ret = filesystem_write(fd, & head->u_st_xpn_server_msg.op_write_mdata_file_size.size, sizeof(ssize_t));
        }

        filesystem_close(fd); //TODO: think if necesary check error in close

cleanup_xpn_server_op_write_mdata_file_size:
        pthread_mutex_unlock( & op_write_mdata_file_size_mutex);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] mutex unlock\n", params->rank);

        req.ret = ret;
        req.server_errno = errno;

        xpn_server_comm_write_data(params->server_type, comm, (char * ) & req, sizeof(struct st_xpn_server_status), rank_client_id, tag_client_id);

        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] write_mdata_file_size(%s, %ld)=%d\n", params->rank, head->u_st_xpn_server_msg.op_write_mdata_file_size.path, head->u_st_xpn_server_msg.op_write_mdata_file_size.size, req.ret);
        debug_info("[Server=%d] [XPN_SERVER_OPS] [xpn_server_op_write_mdata_file_size] << End\n", params->rank);
    }


/* ................................................................... */

