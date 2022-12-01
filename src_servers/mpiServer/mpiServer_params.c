
/*
 *  Copyright 2020-2022 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Luis Miguel Sanchez Garcia, Borja Bergua Guerra
 *
 *  This file is part of mpiServer.
 *
 *  mpiServer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  mpiServer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with mpiServer.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


  /* ... Include / Inclusion ........................................... */

  #include "mpiServer_params.h"


  /* ... Functions / Funciones ......................................... */

  void mpiServer_params_show ( mpiServer_param_st *params )
  {
    printf("MPI servers Current configuration:\n");
    printf("\t-ns <string>:\t%s\n",   params->dns_file) ;
    if(params->thread_mode == TH_POOL){
      printf("\t-tp:\t\tThread Pool Activated\n") ;
    }
    if(params->thread_mode == TH_OP){
      printf("\t-tp:\t\tThread Pool Deactivated (Using Thread per Client)\n") ;
    }
    printf("\t-d <string>:\t%s\n",   params->dirbase) ;
  }
  
  void mpiServer_params_show_usage ( void )
  {
    printf("Usage:\n");
    printf("\t-f <path>: file of servers to be shutdown\n") ;
    printf("\t-ns: file for service name\n") ;
    printf("\t-tp: use thread pool\n") ;
    printf("\t-d <string>: name of the base directory\n") ;
  }
  
  int mpiServer_params_get ( mpiServer_param_st *params, int argc, char *argv[] )
  {
    DEBUG_BEGIN() ;

    // set default values
    params->argc = argc ;
    params->argv = argv ;
    params->size = 0 ;
    params->rank = 0 ;
    params->thread_mode = TH_OP   ;
    strcpy(params->port_name, "") ;
    strcpy(params->srv_name,  "") ;
    strcpy(params->dirbase,   MPISERVER_DIRBASE_DEFAULT) ;
    strcpy(params->dns_file,  "") ;
  
    // update user requests
    for (int i=0; i<argc; i++)
    {
      switch (argv[i][0])
      {
        case '-':
          switch (argv[i][1])
          {
            case 'n':
              if ((strlen(argv[i]) == 3) && (argv[i][2] == 's')){
                strcpy(params->dns_file, argv[i+1]);
                i++;
              }
              break;           
            case 'f':
              strcpy(params->host_file, argv[i+1]);
              i++;
              break;          
            case 'd':
              strcpy(params->dirbase, argv[i+1]);
              i++;
              break;
            case 't':
              if ((strlen(argv[i]) == 3) && (argv[i][2] == 'p')){
                params->thread_mode = TH_POOL;
                i++;
              }
              break;
            case 'h':
              return -1;
  
            default:
              break;
          }
          break;

        default:
          break;      
      }
    }

    // return OK
    DEBUG_END() ;
    return 1;
  }


  /* ................................................................... */
