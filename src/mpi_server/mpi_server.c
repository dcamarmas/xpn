
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


  /* ... Include / Inclusion ........................................... */

    #include "all_system.h"
    #include "base/utils.h"
    #include "mpi_server_params.h"
    #include "mpi_server_ops.h"
    #include "base/workers.h"
    #include "mpi_server_comm.h"
    #include "mpi_server_d2xpn.h"


  /* ... Global variables / Variables globales ......................... */

    char serv_name [HOST_NAME_MAX];
    mpi_server_param_st params;
    worker_t worker;
    int the_end = 0;


  /* ... Auxiliar Functions / Funciones Auxiliares ..................... */

    void mpi_server_run ( struct st_th th )
    {
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_run] >> Begin: OP '%s'; OP_ID %d\n", th.id, mpi_server_op2string(th.type_op), th.type_op);

      mpi_server_do_operation ( &th,  &the_end );

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_run] << End: OP:'%s'\n", th.id, mpi_server_op2string(th.type_op));
    }

    void mpi_server_dispatcher ( struct st_th th )
    {
      int ret ;

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] >> Begin\n", th.id);

      // check params...
      if (NULL == th.params) {
          printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] ERROR: NULL arguments\n", th.id);
          return ;
      }

      struct st_th th_arg;
      int disconnect = 0;

      while (!disconnect)
      {
        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] Waiting for operation\n", th.id);

        ret = mpi_server_comm_read_operation((mpi_server_param_st *) th.params, (MPI_Comm) th.sd, (char *)&(th.type_op), 1, &(th.rank_client_id));
        if (ret < 0) {
            printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] ERROR: read operation fail\n", th.id);
            return;
        }

        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] OP '%s'; OP_ID %d\n", th.id, mpi_server_op2string(th.type_op), th.type_op);

        if (th.type_op == MPI_SERVER_DISCONNECT)
        {
          debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] DISCONNECT received\n", th.id);

          disconnect = 1;
          continue;
        }

        if (th.type_op == MPI_SERVER_FINALIZE)
        {
          debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] FINALIZE received\n", th.id);

          disconnect = 1;
          the_end = 1;
          continue;
        }

        // Launch worker per operation
        th_arg.params   = &params;
        th_arg.sd       = (MPI_Comm) th.sd;
        th_arg.function = mpi_server_run;
        th_arg.type_op  = th.type_op;
        th_arg.rank_client_id = th.rank_client_id;
        th_arg.wait4me  = FALSE;

        base_workers_launch ( &worker, &th_arg, mpi_server_run );

        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] Worker launched\n", th.id);
      }

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] Client %d close\n", th.id, th.rank_client_id);

      mpi_server_comm_disconnect((MPI_Comm)th.sd);

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_dispatcher] End\n", th.id);
    }


  /* ... Functions / Funciones ......................................... */

    int mpi_server_up ( void )
    {
      MPI_Comm     sd ;
      struct st_th th_arg;
      int          ret ;

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] >> Begin\n", 0);

      printf("\n");
      printf(" ----------------\n");
      printf(" Starting servers (%s)\n", serv_name);
      printf(" ----------------\n");
      printf("\n");

      // Initialize
      debug_msg_init() ;

      // mpi_comm initialization
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] mpi_comm initialization\n", 0);
      ret = mpi_server_comm_init(&params) ;
      if (ret < 0) {
          printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] ERROR: mpi_comm initialization fails\n", 0);
          return -1;
      }

      // Workers initialization
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] Workers initialization\n", 0);
      ret = base_workers_init( &worker, params.thread_mode );
      if (ret < 0) {
          printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] ERROR: Workers initialization fails\n", 0);
          return -1;
      }

      // Initialize semaphore for server disks
      /*ret = sem_init(&(params.disk_sem), 0, 1);
      if (ret < 0) {
        printf("[MPI-SERVER] ERROR: semaphore initialize fails\n");
        return -1;
      }*/

      // Initialize semaphore for clients
      /*sprintf(params.sem_name_server, "%s%d", serv_name, getpid());
      sem_t *sem_server = sem_open(params.sem_name_server, O_CREAT, 0777, 1);
      if (sem_server == 0) {
        printf("[MPI-SERVER] ERROR: semaphore open fails\n");
        return -1;
      }*/

      // Loop: receiving + processing
      the_end = 0;
      while (!the_end)
      {
        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] Waiting for accept\n", 0);

        params.client = MPI_COMM_NULL ;

        sd = mpi_server_comm_accept(&params) ;
        if (sd == MPI_COMM_NULL) {
            continue ;
        }

        //Launch dispatcher per aplication
        th_arg.params   = &params;
        th_arg.sd       = sd;
        th_arg.function = mpi_server_dispatcher;
        th_arg.type_op  = 0;
        th_arg.rank_client_id = 0;
        th_arg.wait4me  = FALSE;

        base_workers_launch( &worker, &th_arg, mpi_server_dispatcher );

        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] Dispatcher launched\n", 0);
      }

      // Wait and finalize for all current workers
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] Workers destroy\n", 0);
      base_workers_destroy( &worker );
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] mpi_comm destroy\n", 0);
      mpi_server_comm_destroy( &params );

      // Close semaphores
      /*sem_destroy(&(params.disk_sem));
      sem_unlink(params.sem_name_server);*/

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] >> End\n", 0);

      // return OK
      return 0 ;
    }

    int mpi_server_down ( int argc, char *argv[] )
    {
      int  ret, buf, res = 0;
      char port_name[MPI_MAX_PORT_NAME];
      char srv_name[1024] ;
      MPI_Comm server;
      FILE *file;

      XPN_DEBUG_BEGIN
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] >> Begin\n", 0);

      printf("\n");
      printf(" ----------------\n");
      printf(" Stopping servers (%s)\n", serv_name);
      printf(" ----------------\n");
      printf("\n");

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] MPI_Init\n", 0);
      MPI_Init(&argc, &argv);
      MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

      // Open host file
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] Open host file %s\n", 0, params.shutdown_file);
      file = fopen(params.shutdown_file, "r");
      if (file == NULL) {
          printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] ERROR: invalid file %s\n", 0, params.shutdown_file);
          return -1;
      }

      while (fscanf(file, "%[^\n] ", srv_name) != EOF)
      {
        int version_len;
        char version[MPI_MAX_LIBRARY_VERSION_STRING];
        MPI_Get_library_version(version, &version_len);

        if (strncasecmp(version, "Open MPI", strlen("Open MPI")) != 0)
        {
            debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] ns_lookup server %s\n", 0, srv_name);

            // Lookup port name
            char aux_srv_ip[1024];
            ret = ns_lookup("mpi_server", srv_name, aux_srv_ip, port_name);
            if (ret < 0)
            {
              printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] ERROR: server %s not found\n", 0, srv_name);
              continue;
            }
        }
        else
        {
            debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] MPI_Lookup_name server %s\n", 0, srv_name);

            // Lookup port name on nameserver
            ret = MPI_Lookup_name(srv_name, MPI_INFO_NULL, port_name) ;
            if (MPI_SUCCESS != ret) {
                printf("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] ERROR: server %s not found\n", 0, srv_name);
                continue;
            }
        }

        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] Connect with port %s\n", 0, port_name);

        // Connect with servers
        MPI_Info info;
        MPI_Info_create( &info );
        int errclass, resultlen;
        char err_buffer[MPI_MAX_ERROR_STRING];
        MPI_Info_set(info, "timeout", "1");
        ret = MPI_Comm_connect( port_name, info, 0, MPI_COMM_SELF, &server );
        
        MPI_Error_class(ret,&errclass);
        MPI_Error_string(ret,err_buffer,&resultlen);
        printf("%s", err_buffer);
        MPI_Info_free(&info);
        if (MPI_ERR_PORT == errclass) {
            printf("[MPI-SERVER] Info: Server %s down\n", srv_name) ;
            continue;
        }
	else 
	if (MPI_SUCCESS != errclass) {
            printf("[MPI-SERVER] ERROR: MPI_Comm_connect fails\n") ;
            continue;
        }
        
        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] Send finalize operation\n", 0);
        buf = MPI_SERVER_FINALIZE;
        MPI_Send( &buf, 1, MPI_INT, 0, 0, server );

        debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] Comunicator diconnect\n", 0);
        // MPI_Comm_disconnect( &server ); //TODO: fail
      }

      // Close host file
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] Close host file\n", 0);
      fclose(file);

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] >> End\n", 0);
      // MPI_Finalize();
      XPN_DEBUG_END

      return 0;
    }

    int mpi_server_terminate ( int argc, char *argv[] )
    {
      int  ret, buf, res = 0;
      char port_name[MPI_MAX_PORT_NAME];
      MPI_Comm server;
      
      XPN_DEBUG_BEGIN
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] >> Begin\n", 0);

      printf("\n");
      printf(" ----------------\n");
      printf(" Stopping server (%s)\n", serv_name);
      printf(" ----------------\n");
      printf("\n");

      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_down] MPI_Init\n", 0);
      MPI_Init(&argc, &argv);
      MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

      int version_len;
      char version[MPI_MAX_LIBRARY_VERSION_STRING];
      MPI_Get_library_version(version, &version_len);

      if (strncasecmp(version, "Open MPI", strlen("Open MPI")) != 0)
      {
          // Lookup port name
          char aux_srv_ip[1024];
          ret = ns_lookup("mpi_server", params.srv_name, aux_srv_ip, port_name);
          if (ret < 0) {
              printf("[MPI-SERVER] INFO: server %s not found\n", params.srv_name);
          }
      }
      else
      {
          // Lookup port name on nameserver
          ret = MPI_Lookup_name(params.srv_name, MPI_INFO_NULL, port_name) ;
          if (MPI_SUCCESS != ret) {
              printf("[MPI-SERVER] INFO: server %s not found\n", params.srv_name) ;
          }
      }

      // Connect with servers
      XPN_DEBUG("[MPI-SERVER] Connect to server %s\n", params.srv_name) ;
      MPI_Info info;
      MPI_Info_create( &info );
      int errclass, resultlen;
      char err_buffer[MPI_MAX_ERROR_STRING];
      MPI_Info_set(info, "timeout", "1");
      ret = MPI_Comm_connect( port_name, info, 0, MPI_COMM_SELF, &server );
      
      MPI_Error_class(ret,&errclass);
      MPI_Error_string(ret,err_buffer,&resultlen);
      XPN_DEBUG("%s", err_buffer);
      MPI_Info_free(&info);
      if (MPI_ERR_PORT == errclass){
          printf("[MPI-SERVER] Info: Server %s down\n", params.srv_name) ;
      }
      if (MPI_SUCCESS != errclass) {
          printf("[MPI-SERVER] ERROR: MPI_Comm_connect fails\n") ;
      }
      
      XPN_DEBUG("[MPI-SERVER] SEND: MPI_SERVER_FINALIZE to server %s\n", params.srv_name) ;
      buf = MPI_SERVER_FINALIZE;
      MPI_Send( &buf, 1, MPI_INT, 0, 0, server );

      XPN_DEBUG("[MPI-SERVER] SEND finish: MPI_SERVER_FINALIZE to server %s\n", params.srv_name) ;
      MPI_Comm_disconnect( &server ); 
      
      XPN_DEBUG_END
      debug_info("[TH_ID=%d] [MPI_SERVER] [mpi_server_up] >> End\n", 0);
      MPI_Finalize();

      return 0;
    }


    /*
     * Main
     */

    int main ( int argc, char *argv[] )
    {
      int    ret = -1 ;
      char * exec_name = NULL ;

      // Initializing...
      setbuf(stdout, NULL);
      setbuf(stderr, NULL);

      // Welcome...
      printf("\n") ;
      printf(" mpi_server\n") ;
      printf(" ----------\n") ;
      printf("\n") ;
      printf(" Begin.\n") ;
      printf("\n") ;

      // Get arguments..
      debug_info("[TH_ID=%d] [MPI_SERVER] [main] Get server params\n", 0);
      ret = mpi_server_params_get(&params, argc, argv) ;
      if (ret < 0) {
        mpi_server_params_show_usage() ;
        return -1;
      }

      // Show configuration...
      exec_name = basename(argv[0]);
      printf(" * action=%s\n", exec_name);
      gethostname(serv_name, HOST_NAME_MAX);
      printf(" * host=%s\n",   serv_name);
      mpi_server_params_show(&params);

      // Do associate action...
      if (strcasecmp(exec_name, "xpn_stop_mpi_server") == 0)
      {
          debug_info("[TH_ID=%d] [MPI_SERVER] [main] Down servers\n", 0);
          ret = mpi_server_down (argc, argv);
      }
      else if (strcasecmp(exec_name, "xpn_terminate_mpi_server") == 0)
      {
          debug_info("[TH_ID=%d] [MPI_SERVER] [main] Terminate server\n", 0);
          ret = mpi_server_terminate (argc, argv);
      }
      else
      {
          debug_info("[TH_ID=%d] [MPI_SERVER] [main] Up servers\n", 0);
          ret = mpi_server_up ();
      }

      return ret;
    }


 /* ................................................................... */

