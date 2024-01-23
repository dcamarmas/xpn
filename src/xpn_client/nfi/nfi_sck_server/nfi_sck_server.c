
/*
 *  Copyright 2020-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos
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

#include "nfi_sck_server.h"


/* ... Const / Const ................................................. */

#define FILESYSTEM_DLSYM 1


/* ... Global variables / Variables globales ........................ */


/* ... Auxiliar Functions / Funciones Auxiliares ..................... */

//Communication
int sck_server_write_operation(int sd, struct st_sck_server_msg * head)
{
  int ret;

  debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] >> Begin\n", head->id);

  debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] Send operation\n", head->id);

  ret = sck_client_write_operation(sd, (char *)&(head->type), 1, head->id);
  if (ret < 0)
  {
    printf("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] ERROR: sck_client_write_operation fails\n", head->id);
    return -1;
  }

  debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] Execute operation: %d -> \n", head->id, head->type);

  switch (head->type)
  {
    //File API
    case SCK_SERVER_OPEN_FILE_WS:
    case SCK_SERVER_OPEN_FILE_WOS:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] OPEN operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_open, sizeof(struct st_sck_server_open), head->id);
      break;
    case SCK_SERVER_CREAT_FILE_WS:
    case SCK_SERVER_CREAT_FILE_WOS:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] CREAT operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_creat, sizeof(struct st_sck_server_creat), head->id);
      break;
    case SCK_SERVER_READ_FILE_WS:
    case SCK_SERVER_READ_FILE_WOS:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] READ operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_read, sizeof(struct st_sck_server_read), head->id);
      break;
    case SCK_SERVER_WRITE_FILE_WS:
    case SCK_SERVER_WRITE_FILE_WOS:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] WRITE operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_write, sizeof(struct st_sck_server_write), head->id);
      break;
    case SCK_SERVER_CLOSE_FILE_WS:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] CLOSE operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_close, sizeof(struct st_sck_server_close), head->id);
      break;
    case SCK_SERVER_RM_FILE:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] RM operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_rm, sizeof(struct st_sck_server_rm), head->id);
      break;
    case SCK_SERVER_RM_FILE_ASYNC:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] RM_ASYNC operation\n", head->id);
      ret = sck_client_write_data(sd, (char *)&head->u_st_sck_server_msg.op_rm, sizeof(struct st_sck_server_rm), head->id);
      break;
    case SCK_SERVER_RENAME_FILE:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] RENAME operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_rename, sizeof(struct st_sck_server_rename), head->id);
      break;
    case SCK_SERVER_GETATTR_FILE:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] GETATTR operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_getattr, sizeof(struct st_sck_server_getattr), head->id);
      break;

    //Directory API
    case SCK_SERVER_MKDIR_DIR:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] MDKIR operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_mkdir, sizeof(struct st_sck_server_mkdir), head->id);
      break;
    case SCK_SERVER_OPENDIR_DIR:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] OPENDIR operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_opendir, sizeof(struct st_sck_server_opendir), head->id);
      break;
    case SCK_SERVER_READDIR_DIR:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] READDIR operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_readdir, sizeof(struct st_sck_server_readdir), head->id);
      break;
    case SCK_SERVER_CLOSEDIR_DIR:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] CLOSEDIR operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_closedir, sizeof(struct st_sck_server_closedir), head->id);
      break;
    case SCK_SERVER_RMDIR_DIR:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] RMDIR operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_rmdir, sizeof(struct st_sck_server_rmdir), head->id);
      break;
    case SCK_SERVER_RMDIR_DIR_ASYNC:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] RMDIR_ASYNC operation\n", head->id);
      ret = sck_client_write_data(sd, (char *)&head->u_st_sck_server_msg.op_rmdir, sizeof(struct st_sck_server_rmdir), head->id);
      break;

    //Optimization API
    case SCK_SERVER_FLUSH_FILE:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] FLUSH operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_flush, sizeof(struct st_sck_server_flush), head->id);
      break;
    case SCK_SERVER_PRELOAD_FILE:
      debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] PRELOAD operation\n", head->id);
      ret = sck_client_write_data(sd, (char * )&head->u_st_sck_server_msg.op_preload, sizeof(struct st_sck_server_preload), head->id);
      break;
  }

  debug_info("[SERV_ID=%s] [NFI_SCK] [sck_server_write_operation] >> End\n", head->id);

  return ret;
}

int nfi_sck_server_doRequest(struct nfi_sck_server_server * server_aux, struct st_sck_server_msg * msg, char * req, int req_size)
{
  ssize_t ret;

  debug_info("[SERV_ID=%s] [NFI_SCK] [nfi_sck_server_doRequest] >> Begin\n", server_aux->id);

  // send request...
  debug_info("[SERV_ID=%s] [NFI_SCK] [nfi_sck_server_doRequest] Send operation: %s\n", server_aux->id, msg->id);

  ret = sck_server_write_operation(server_aux->params.server, msg);
  if (ret < 0) {
    return -1;
  }

  // read response...
  debug_info("[SERV_ID=%s] [NFI_SCK] [nfi_sck_server_doRequest] Response operation: %s\n", server_aux->id, msg->id);

  ret = sck_client_read_data(server_aux->params.server, req, req_size, msg->id);
  if (ret < 0) {
    return -1;
  }

  debug_info("[SERV_ID=%s] [NFI_SCK] [nfi_sck_server_doRequest] >> End\n", server_aux->id);

  return 0;
}

int nfi_sck_server_keepConnected(struct nfi_server * serv)
{
  debug_info("[SERV_ID=%d] [NFI_SCK] [NFI_SCK_server_keep_connected] >> Begin\n", serv->id);

  if (NULL == serv) {
    return -1;
  }

#ifdef NFI_DYNAMIC
  if (serv->private_info == NULL)
  {
    debug_info("[SERV_ID=%d] [NFI_SCK] [NFI_SCK_server_keep_connected] Server reconnect\n", server_aux->id);

    int ret = nfi_sck_server_reconnect(serv);
    if (ret < 0)
    {
      serv->private_info = NULL;
      return -1;
    }
  }
#endif

  debug_info("[SERV_ID=%d] [NFI_SCK] [NFI_SCK_server_keep_connected] >> End\n", serv->id);

  return (serv->private_info != NULL);
}

void nfi_2_sck_server_attr(struct stat * att, struct nfi_attr * nfi_att)
{
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_2_sck_server_attr] >> Begin\n", -1);

  att->st_dev = nfi_att->st_dev;
  att->st_ino = nfi_att->st_ino;

  if (nfi_att->at_type == NFIFILE) {
    att->st_mode = nfi_att->at_mode | S_IFREG; // protection
  }

  if (nfi_att->at_type == NFIDIR) {
    att->st_mode = nfi_att->at_mode | S_IFDIR; // protection
  }

  att->st_nlink   = nfi_att->at_nlink;   // number of hard links
  att->st_uid     = nfi_att->at_uid;     // user ID of owner
  att->st_gid     = nfi_att->at_gid;     // group ID of owner
  att->st_size    = nfi_att->at_size;    // size
  att->st_blksize = nfi_att->at_blksize; // blocksize for filesystem I/O
  att->st_blocks  = nfi_att->at_blocks;  // number of blocks allocated
  att->st_atime   = nfi_att->at_atime;   // time of last access
  att->st_mtime   = nfi_att->at_mtime;   // time of last modification
  att->st_ctime   = nfi_att->at_ctime;   // time of last change

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_2_sck_server_attr] >> End\n", -1);
}

void sck_server_2_nfi_attr(struct nfi_attr * nfi_att, struct stat * att)
{
  debug_info("[SERV_ID=%d] [NFI_SCK] [sck_server_2_nfi_attr] >> Begin\n", -1);

  nfi_att->st_dev = att->st_dev;
  nfi_att->st_ino = att->st_ino;

  if (S_ISREG(att->st_mode)) {
    nfi_att->at_type = NFIFILE;
  }
  if (S_ISDIR(att->st_mode)) {
    nfi_att->at_type = NFIDIR;
  }

  nfi_att->at_mode    = att->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // protection
  nfi_att->at_nlink   = att->st_nlink;                                // number of hard links
  nfi_att->at_uid     = att->st_uid;                                  // user ID of owner
  nfi_att->at_gid     = att->st_gid;                                  // group ID of owner
  nfi_att->at_size    = att->st_size;                                 // total size, in bytes
  nfi_att->at_blksize = att->st_blksize;                              // blocksize for filesystem I/O
  nfi_att->at_blocks  = att->st_blocks;                               // number of blocks allocated
  nfi_att->at_atime   = att->st_atime;                                // time of last access
  nfi_att->at_mtime   = att->st_mtime;                                // time of last modification
  nfi_att->at_ctime   = att->st_ctime;                                // time of last change

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_2_sck_server_attr] >> End\n", -1);
}

void SCK_SERVERtoNFIInfo(__attribute__((__unused__)) struct nfi_info * nfi_inf, __attribute__((__unused__)) struct nfi_info * sck_server_inf)
{
  debug_info("[SERV_ID=%d] [NFI_SCK] [sck_server_2_nfi_info] >> Begin\n", -1);

  //TODO

  debug_info("[SERV_ID=%d] [NFI_SCK] [sck_server_2_nfi_info] >> End\n", -1);
}


/* ... Functions / Funciones ......................................... */

int nfi_sck_server_init(char * url, struct nfi_server * serv, __attribute__((__unused__)) struct nfi_attr_server * attr)
{
  int ret;
  char server[PATH_MAX], dir[PATH_MAX], prt[PATH_MAX];
  struct nfi_sck_server_server * server_aux;

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] >> Begin\n", serv->id);

  // check params...
  if (serv == NULL)
  {
    printf("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] ERROR: serv argument is NULL\n", serv->id);
    return -1;
  }

  // new nfi_ops with sck_server functions...
  serv->ops = (struct nfi_ops *)malloc(sizeof(struct nfi_ops));
  NULL_RET_ERR(serv->ops, SCK_SERVER_ERR_MEMORY);

  // Fill serv->ops...
  bzero(serv->ops, sizeof(struct nfi_ops));
  serv->ops->nfi_reconnect = nfi_sck_server_reconnect;
  serv->ops->nfi_disconnect = nfi_sck_server_disconnect;

  serv->ops->nfi_open = nfi_sck_server_open;
  serv->ops->nfi_create = nfi_sck_server_create;
  serv->ops->nfi_read = nfi_sck_server_read;
  serv->ops->nfi_write = nfi_sck_server_write;
  serv->ops->nfi_close = nfi_sck_server_close;
  serv->ops->nfi_remove = nfi_sck_server_remove;
  serv->ops->nfi_rename = nfi_sck_server_rename;
  serv->ops->nfi_getattr = nfi_sck_server_getattr;
  serv->ops->nfi_setattr = nfi_sck_server_setattr;

  serv->ops->nfi_opendir = nfi_sck_server_opendir;
  serv->ops->nfi_mkdir = nfi_sck_server_mkdir;
  serv->ops->nfi_readdir = nfi_sck_server_readdir;
  serv->ops->nfi_closedir = nfi_sck_server_closedir;
  serv->ops->nfi_rmdir = nfi_sck_server_rmdir;

  serv->ops->nfi_preload = nfi_sck_server_preload;
  serv->ops->nfi_flush = nfi_sck_server_flush;

  serv->ops->nfi_statfs = nfi_sck_server_statfs;

  // parse url...
  ret = ParseURL(url, prt, NULL, NULL, server, NULL, dir);
  if (ret < 0)
  {
    printf("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] ERROR: incorrect url '%s'.\n", serv->id, url);
    sck_server_err(SCK_SERVER_ERR_URL);
    FREE_AND_NULL(serv->ops);
    return -1;
  }

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] ParseURL(%s)= %s; %s\n", serv->id, url, server, dir);

  // new nfi_sckserver_server...
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Copy nfi_sck_server_server structure\n", serv->id);

  server_aux = (struct nfi_sck_server_server * )malloc(sizeof(struct nfi_sck_server_server));
  if (server_aux == NULL)
  {
    printf("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] ERROR: out of memory.\n", serv->id);
    FREE_AND_NULL(serv->ops);
    return -1;
  }

  // private_info...
  serv->private_info = (void *)server_aux;

  // Initialize params
  memset( & (server_aux->params), 0, sizeof(sck_client_param_st));

  // thread mode
  server_aux->params.xpn_thread = TH_NOT;
  serv->xpn_thread = TH_NOT;
  char * env_thread = getenv("XPN_THREAD");
  if (env_thread != NULL)
  {
    server_aux->params.xpn_thread = atoi(env_thread);
    serv->xpn_thread = atoi(env_thread);
  }

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Thread mode: %d\n", serv->id, serv->xpn_thread);

  // Session mode
  server_aux->params.xpn_session = 0;
  char * env_session = getenv("XPN_SESSION");
  if (env_session != NULL) {
    server_aux->params.xpn_session = atoi(env_session);
  }

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Sesion mode: %d\n", serv->id, server_aux->params.xpn_session);

  // Locality mode
  server_aux->params.xpn_locality = 1;
  char * env_locality = getenv("XPN_LOCALITY");
  if (env_locality != NULL) {
    server_aux->params.xpn_locality = atoi(env_locality);
  }

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Locality enable: %d\n", serv->id, server_aux->params.xpn_locality);

  // Initialize SCK Client communication side...
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Initialize SCK Client communication side\n", serv->id);

  ret = sck_client_comm_init(&(server_aux->params));
  if (ret < 0)
  {
    FREE_AND_NULL(serv->ops);
    FREE_AND_NULL(server_aux);
    return -1;
  }

  // copy 'url' string...
  serv->url = strdup(url);
  NULL_RET_ERR(serv->url, SCK_SERVER_ERR_MEMORY);

  // new server wrk...
  serv->wrk = (struct nfi_worker *)malloc(sizeof(struct nfi_worker));
  memset(serv->wrk, 0, sizeof(struct nfi_worker));
  serv->wrk->server = serv;

  // Initialize workers
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Initialize workers\n", serv->id);

  ret = nfiworker_init(serv);

  // Server conection
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Server conection\n", serv->id);

  ret = nfi_sck_server_connect(serv, url, prt, server, dir);
  if (ret < 0) 
  {
    FREE_AND_NULL(serv->ops);
    FREE_AND_NULL(server_aux);
    return -1;
  }

  // Data locality
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] Data locality\n", serv->id);

  ret = sck_client_comm_locality( & (server_aux->params));
  if (ret < 0)
  {
    FREE_AND_NULL(serv->ops);
    FREE_AND_NULL(server_aux);
    return -1;
  }
  
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] << End\n", serv->id);

  return 0;
}

int nfi_sck_server_destroy(struct nfi_server * serv)
{
  int ret;
  struct nfi_sck_server_server * server_aux;

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_destroy] >> Begin\n", serv->id);

  // check params...
  if (serv == NULL) {
    return 0;
  }

  // private_info...
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_destroy] Get server private info\n", serv->id);

  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  if (server_aux == NULL) {
    return 0;
  }

  // Workers destroy...
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_destroy] Destroy workers\n", serv->id);

  nfiworker_destroy(serv);

  // SCK Finalize...
  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_destroy] Destroy SCK Client communication\n", serv->id);

  ret = sck_client_comm_destroy( & (server_aux->params));
  if (ret < 0) {
    printf("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_init] ERROR: sck_client_comm_destroy fails\n", serv->id);
  }

  // free private_info, 'url' string and 'server' string...
  FREE_AND_NULL(serv->ops);
  FREE_AND_NULL(serv->private_info);
  FREE_AND_NULL(serv->url);
  FREE_AND_NULL(serv->server);

  debug_info("[SERV_ID=%d] [NFI_SCK] [nfi_sck_server_destroy] << End\n", serv->id);

  return 0;
}

int nfi_sck_server_connect(struct nfi_server * serv, __attribute__((__unused__)) char * url, __attribute__((__unused__)) char * prt, char * server, __attribute__((__unused__)) char * dir)
{
  int ret;
  struct nfi_sck_server_server * server_aux;

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_connect] >> Begin\n", serv->id);

  // check params...
  if (serv == NULL) {
    return -1;
  }

  // private_info...
  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_connect] Get server private info\n", serv->id);

  server_aux = (struct nfi_sck_server_server * )(serv->private_info);
  if (server_aux == NULL) {
    return -1;
  }

  // Copy server name
  strcpy(server_aux->params.srv_name, server);

  // MPI Connection...
  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_connect] MPI server conection\n", serv->id);

  ret = sck_client_comm_connect( & (server_aux->params));
  if (ret < 0) {
    return -1;
  }

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_connect] << End\n", serv->id);

  return 0;
}

int nfi_sck_server_disconnect(struct nfi_server * serv)
{
  int ret;
  struct nfi_sck_server_server * server_aux;

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_disconnect] >> Begin\n", serv->id);

  // check params...
  if (serv == NULL) {
    return 0;
  }

  // private_info...
  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_disconnect] Get server private info\n", serv->id);

  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  if (server_aux == NULL) {
    return 0;
  }

  // MPI Disconnect...
  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_disconnect] MPI server disconnect\n", serv->id);

  ret = sck_client_comm_disconnect( & (server_aux->params));
  if (ret < 0) {
    printf("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] ERROR: mpi_client_comm_disconnect fails\n", serv->id);
  }

  FREE_AND_NULL(serv->private_info);

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_disconnect] << End\n", serv->id);

  return 0;
}

int nfi_sck_server_reconnect(struct nfi_server * serv)
{
  int ret;
  char server[PATH_MAX], dir[PATH_MAX];
  struct nfi_sck_server_server * server_aux;

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] >> Begin\n", serv->id);

  // Check arguments
  if (serv == NULL) {
    return -1;
  }

  // ParseURL...
  ret = ParseURL(serv->url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0)
  {
    printf("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] ERROR: url %s incorrect.\n", serv->id, serv->url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] ParseURL(%s)= %s; %s\n", serv->id, serv->url, server, dir);

  // new nfi_mpiserver_server...
  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] Copy nfi_mpi_server_server structure\n", serv->id);

  server_aux = (struct nfi_sck_server_server *)malloc(sizeof(struct nfi_sck_server_server));
  NULL_RET_ERR(server_aux, SCK_SERVER_ERR_MEMORY);

  strcpy(server_aux->path, dir);
  serv->private_info = (void *)server_aux;

  // reconnect...
  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] MPI server conection\n", serv->id);

  ret = sck_client_comm_connect( & (server_aux->params));
  if (ret < 0)
  {
    debug_error("nfi_sck_server_reconnect: sck_client_comm_connect fails.\n");
    return -1;
  }

  debug_info("[SERV_ID=%d] [NFI_MPI] [nfi_mpi_server_reconnect] << End\n", serv->id);

  return 0;
}

// File API
int nfi_sck_server_open(struct nfi_server * serv, char * url, struct nfi_fhandle * fho) {
  int ret;
  char dir[PATH_MAX], server[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fho, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_open(ID=%s): begin %s\n", server_aux->id, url);

  // from url->server + dir
  ret = ParseURL(url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_open: url %s incorrect.\n", url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  fho->url = strdup(url);
  NULL_RET_ERR(fho->url, SCK_SERVER_ERR_MEMORY);

  fh_aux = (struct nfi_sck_server_fhandle *)malloc(sizeof(struct nfi_sck_server_fhandle));
  if (fh_aux == NULL) {
    sck_server_err(SCK_SERVER_ERR_MEMORY);
    FREE_AND_NULL(fho->url);
    return -1;
  }

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    fh_aux->fd = real_posix_open2(path, O_RDWR, S_IRWXU);
    if (fh_aux->fd < 0) {
      debug_error("real_posix_open fails to open '%s' in server %s.\n", dir, serv->server);
      FREE_AND_NULL(fh_aux);
      FREE_AND_NULL(fho->url);
      return -1;
    }
    if (server_aux->params.xpn_session == 0) {
      real_posix_close(fh_aux->fd);
    }
    strcpy(fh_aux->path, dir);
  }
  /************** REMOTE ****************/
  else {
    if (server_aux->params.xpn_session) {
      msg.type = SCK_SERVER_OPEN_FILE_WS;
    } else {
      msg.type = SCK_SERVER_OPEN_FILE_WOS;
    }
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_open.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (fh_aux->fd), sizeof(int));

    if (fh_aux->fd < 0) {
      debug_error("real_posix_open fails to open '%s' in server %s.\n", dir, serv->server);
      FREE_AND_NULL(fh_aux);
      FREE_AND_NULL(fho->url);
      return -1;
    }

    memccpy(fh_aux->path, dir, 0, PATH_MAX - 1);

  }
  /*****************************************/

  fho->type = NFIFILE;
  fho->server = serv;
  fho->priv_fh = (void *)fh_aux;

  DEBUG_END();

  return 0;
}



int nfi_sck_server_create(struct nfi_server * serv, char * url, struct nfi_attr * attr, struct nfi_fhandle * fh) {
  int ret;
  char server[PATH_MAX], dir[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;
  struct st_sck_server_attr_req req;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(attr, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_create(ID=%s): begin %s\n", server_aux->id, url);

  // url->server + dir
  ret = ParseURL(url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "ERROR: nfi_sck_server_create: url %s incorrect.\n", url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  // private_info file handle
  fh_aux = (struct nfi_sck_server_fhandle *)malloc(sizeof(struct nfi_sck_server_fhandle));
  NULL_RET_ERR(fh_aux, SCK_SERVER_ERR_MEMORY);
  bzero(fh_aux, sizeof(struct nfi_sck_server_fhandle));

  // create the file into the directory

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    fh_aux->fd = real_posix_open2(path, O_CREAT | O_RDWR | O_TRUNC, attr->at_mode);
    if (fh_aux->fd < 0) {
      debug_error("files_posix_open fails to creat '%s' in server '%s'.\n", dir, serv->server);
      FREE_AND_NULL(fh_aux);
      DEBUG_END();
      return -1;
    }

    //Get stat
    ret = real_posix_stat(path, & (req.attr));
    if (ret < 0) {
      debug_error("nfi_sck_server_create: Fail stat %s.\n", dir);
      FREE_AND_NULL(fh_aux);
      DEBUG_END();
      return ret;
    }

    if (server_aux->params.xpn_session == 0) {
      real_posix_close(fh_aux->fd);
    }
    strcpy(fh_aux->path, dir);
  }
  /************** REMOTE ****************/
  else {
    if (server_aux->params.xpn_session) {
      msg.type = SCK_SERVER_CREAT_FILE_WS;
    } else {
      msg.type = SCK_SERVER_CREAT_FILE_WOS;
    }
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_creat.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (fh_aux->fd), sizeof(int));

    if (fh_aux->fd < 0) {
      debug_error("doRequest(creat) fails to open '%s' in server %s.\n", dir, serv->server);
      FREE_AND_NULL(fh_aux);
      DEBUG_END();
      return -1;
    }

    memccpy(fh_aux->path, dir, 0, PATH_MAX - 1);

    // Get stat
    //bzero(&msg, sizeof(struct st_sck_server_msg));
    msg.type = SCK_SERVER_GETATTR_FILE;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_getattr.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& req, sizeof(struct st_sck_server_attr_req));
  }
  /*****************************************/

  debug_info("CREATE ------------------- %s\n", server_aux->params.server_name);

  fh->type = NFIFILE;
  fh->server = serv;
  fh->priv_fh = (void *)fh_aux;

  fh->url = strdup(url);
  if (fh->url == NULL) {
    sck_server_err(SCK_SERVER_ERR_MEMORY);
    FREE_AND_NULL(fh_aux);
    DEBUG_END();
    return -1;
  }

  sck_server_2_nfi_attr(attr, & req.attr);

  DEBUG_END();

  return 0;
}



ssize_t nfi_sck_server_read(struct nfi_server * serv, struct nfi_fhandle * fh, void * buffer, off_t offset, size_t size) {
  int ret, cont, diff;
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;
  struct st_sck_server_read_req req;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_read(%s): begin off %d size %d\n", server_aux->id, (int) offset, (int) size);
  fh_aux = (struct nfi_sck_server_fhandle *)fh->priv_fh;

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    if (server_aux->params.xpn_session) {
      real_posix_lseek(fh_aux->fd, offset, SEEK_SET);
      //if(server_aux->params.sem_server != 0) sem_wait(server_aux->params.sem_server);
      ret = real_posix_read(fh_aux->fd, buffer, size);
      //if(server_aux->params.sem_server != 0) sem_post(server_aux->params.sem_server);

      debug_info("[NFI-SCK-CLI] read %s(%d) off %ld size %zu (ret:%zd)", fh->url, fh_aux->fd, (long int) offset, size, ret);
    } 
    else {
      int fd;
      char path [PATH_MAX];

      strcpy(path, server_aux->params.dirbase);
      strcat(path, "/");
      strcat(path, fh_aux->path);

      fd = real_posix_open(path, O_RDONLY);
      if (fd < 0) {
        debug_error("real_posix_read reads zero bytes from url:%s offset:%ld size:%zu (ret:%zd) errno=%d\n", fh->url, (long int) offset, size, ret, errno);
        return -1;
      }

      real_posix_lseek(fd, offset, SEEK_SET);
      //if(server_aux->params.sem_server != 0) sem_wait(server_aux->params.sem_server);
      ret = real_posix_read(fd, buffer, size);
      //if(server_aux->params.sem_server != 0) sem_post(server_aux->params.sem_server);

      debug_info("[NFI-SCK-CLI] read %s(%d) off %ld size %zu (ret:%zd)", fh->url, fd, (long int) offset, size, ret);

      real_posix_close(fd);
    }

    if (ret < 0) {
      debug_error("real_posix_read reads zero bytes from url:%s offset:%ld size:%zu (ret:%zd) errno=%d\n", fh->url, (long int) offset, size, ret, errno);
      return -1;
    }
  }
  /************** REMOTE ****************/
  else {
    // send read request
    if (server_aux->params.xpn_session) {
      msg.type = SCK_SERVER_READ_FILE_WS;
      msg.u_st_sck_server_msg.op_read.fd = fh_aux->fd;
      debug_info("[NFI-SCK-CLI] read:->fd     %d \n", msg.u_st_sck_server_msg.op_read.fd);
    } else {
      msg.type = SCK_SERVER_READ_FILE_WOS;
      memccpy(msg.u_st_sck_server_msg.op_read.path, fh_aux->path, 0, PATH_MAX);
      debug_info("[NFI-SCK-CLI] read:->path   %s \n", msg.u_st_sck_server_msg.op_read.path);
    }
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    msg.u_st_sck_server_msg.op_read.offset = offset;
    msg.u_st_sck_server_msg.op_read.size = size;

    debug_info("[NFI-SCK-CLI] read:->offset %d \n", (int) msg.u_st_sck_server_msg.op_read.offset);
    debug_info("[NFI-SCK-CLI] read:->size   %d \n", msg.u_st_sck_server_msg.op_read.size);

    ret = sck_server_write_operation(server_aux->params.server, & msg);
    if (ret < 0) {
      fprintf(stderr, "ERROR: (1)nfi_sck_server_read: Error on write operation\n");
      return -1;
    }

    // read n times: number of bytes + read data (n bytes)
    cont = 0;
    do {
      ret = sck_client_read_data(server_aux->params.server, (char *)& req, sizeof(struct st_sck_server_read_req), msg.id);
      debug_info("[NFI-SCK-CLI] nfi_sck_server_read(ID=%s): (1)sck_client_read_data = %d.\n", server_aux->id, ret);

      if (ret < 0) {
        fprintf(stderr, "ERROR: (2)nfi_sck_server_read: Error on write operation\n");
        return -1;
      }

      // TODO: sck_server_ops.c:465 ->if (req.size < 0)->error on server side so... something must be done on client side

      if (req.size > 0) {
        debug_info("[NFI-SCK-CLI] nfi_sck_server_read(ID=%s): (2)sck_client_read_data = %d. size = %d\n", server_aux->id, ret, req.size);
        ret = sck_client_read_data(server_aux->params.server, (char *)buffer + cont, req.size, msg.id);
        debug_info("[NFI-SCK-CLI] nfi_sck_server_read(ID=%s): (2)sck_client_read_data = %d.\n", server_aux->id, ret);

        if (ret < 0) {
          fprintf(stderr, "ERROR: (3)nfi_sck_server_read: Error on read operation\n");
        }
      }

      cont = cont + req.size;
      diff = msg.u_st_sck_server_msg.op_read.size - cont;

    } while ((diff > 0) && (req.size != 0));

    if (req.size < 0) {
      fprintf(stderr, "ERROR: nfi_sck_server_read: Fail read %s off %d size %d (err:%d).\n", fh->url, (int) offset, (int) size, (int) req.size);
      sck_server_err(SCK_SERVER_ERR_READ);
      return -1;
    }

    ret = cont;
  }

  DEBUG_END();

  return ret;
}


ssize_t nfi_sck_server_write(struct nfi_server * serv, struct nfi_fhandle * fh, void * buffer, off_t offset, size_t size)
{
  int ret = -1; 
  int diff, cont;

  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;
  struct st_sck_server_write_req req;

  DEBUG_BEGIN();

  // Check arguments...
  if (size == 0) {
    return 0;
  }

  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_write(ID=%s): begin off %d size %d\n", server_aux->id, (int) offset, (int) size);
  fh_aux = (struct nfi_sck_server_fhandle *)fh->priv_fh;

  /************** LOCAL *****************/
  if (server_aux->params.locality) 
  {
    if (server_aux->params.xpn_session) 
    {
      real_posix_lseek(fh_aux->fd, offset, SEEK_SET);
      //if(server_aux->params.sem_server != 0) sem_wait(server_aux->params.sem_server);
      ret = real_posix_write(fh_aux->fd, buffer, size);
      //if(server_aux->params.sem_server != 0) sem_post(server_aux->params.sem_server);

      debug_info("[NFI-SCK-CLI] write %s(%d) off %ld size %zu (ret:%zd)", fh->url, fh_aux->fd, (long int) offset, size, ret);
    } else 
    {
      int fd;

      fd = real_posix_open(fh_aux->path, O_WRONLY); // WOS
      if (fd < 0) {
        debug_error("real_posix_write writes zero bytes from url:%s offset:%ld size:%zu (ret:%zd) errno=%d\n", fh->url, (long int) offset, size, ret, errno);
        return -1;
      }

      real_posix_lseek(fd, offset, SEEK_SET);
      //if(server_aux->params.sem_server != 0) sem_wait(server_aux->params.sem_server);
      ret = real_posix_write(fd, buffer, size);
      //if(server_aux->params.sem_server != 0) sem_post(server_aux->params.sem_server);

      debug_info("[NFI-SCK-CLI] write %s(%d) off %ld size %zu (ret:%zd)", fh->url, fd, (long int) offset, size, ret);

      real_posix_close(fd); // WOS
    }

    if (ret < 0) {
      debug_error("real_posix_write writes zero bytes from url:%s offset:%ld size:%zu (ret:%zd) errno=%d\n", fh->url, (long int) offset, size, ret, errno);
      return -1;
    }
  }
  /************** REMOTE ****************/
  else 
  {
    // send write request
    if (server_aux->params.xpn_session) 
    {
      msg.type = SCK_SERVER_WRITE_FILE_WS;
      msg.u_st_sck_server_msg.op_write.fd = fh_aux->fd;
      debug_info("[NFI-SCK-CLI] write:->fd     %d \n", msg.u_st_sck_server_msg.op_write.fd);
    } else 
    {
      msg.type = SCK_SERVER_WRITE_FILE_WOS;
      memccpy(msg.u_st_sck_server_msg.op_write.path, fh_aux->path, 0, PATH_MAX - 1);
      debug_info("[NFI-SCK-CLI] write:->path   %s \n", msg.u_st_sck_server_msg.op_write.path);
    }

    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    msg.u_st_sck_server_msg.op_write.offset = offset;
    msg.u_st_sck_server_msg.op_write.size = size;

    debug_info("[NFI-SCK-CLI] write:->offset %d \n", (int) msg.u_st_sck_server_msg.op_write.offset);
    debug_info("[NFI-SCK-CLI] write:->size   %d \n", msg.u_st_sck_server_msg.op_write.size);

    ret = sck_server_write_operation(server_aux->params.server, & msg);
    if (ret < 0) 
    {
      fprintf(stderr, "(1)ERROR: nfi_sck_server_write(ID=%s): Error on write operation\n", server_aux->id);
      return -1;
    }

    // write n times: ...
    diff = size;
    cont = 0;

    // Max buffer size
    int buffer_size = size;
    if (buffer_size > MAX_BUFFER_SIZE) 
    {
      buffer_size = MAX_BUFFER_SIZE;
    }

    do 
    {
      int bytes_to_write = 0;

      if( diff > buffer_size)       bytes_to_write = buffer_size;
      else                            bytes_to_write = diff;
      
      ret = sck_client_write_data(server_aux->params.server, (char *)buffer + cont, bytes_to_write, msg.id); 
      
      if (ret < 0) 
      {
        fprintf(stderr, "(2)ERROR: nfi_sck_server_write(ID=%s): Error on write operation\n", server_aux->id);
      }

      cont = cont + bytes_to_write; //Send bytes
      diff = size - cont;

    } while ((diff > 0) && (ret != 0));

    ret = sck_client_read_data(server_aux->params.server, (char *)& req, sizeof(struct st_sck_server_write_req), msg.id);
    if (ret < 0) 
    {
      fprintf(stderr, "(3)ERROR: nfi_sck_server_write(ID=%s): Error on write operation\n", server_aux->id);
      return -1;
    }

    debug_info("[NFI-SCK-CLI] nfi_sck_server_write(ID=%s): write %s off %d size %d (err:%d).\n", server_aux->id, fh->url, (int) offset, (int) size, (int) req.size);
    if (req.size < 0) 
    {
      fprintf(stderr, "ERROR: nfi_sck_server_write(ID=%s): Fail write %s off %d size %d (err:%d).\n", server_aux->id, fh->url, (int) offset, (int) size, (int) req.size);
      sck_server_err(SCK_SERVER_ERR_WRITE);
      return -1;
    }

    ret = cont;
  }

  DEBUG_END();

  return ret;
}


int nfi_sck_server_close(struct nfi_server * serv, struct nfi_fhandle * fh) {
  int ret = -1;
  struct nfi_sck_server_fhandle * fh_aux;
  struct nfi_sck_server_server * server_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_close(ID=%s): begin\n", server_aux->id);

  // without session->just return ok
  if (server_aux->params.xpn_session == 0) {
    debug_info("[NFI-SCK-CLI] nfi_sck_server_close(ID=%s): end\n", server_aux->id);
    return 1;
  }

  if (NULL == fh->priv_fh) {
    debug_info("[NFI-SCK-CLI] nfi_sck_server_close(ID=%s): end\n", server_aux->id);
    return -1;
  }

  fh_aux = (struct nfi_sck_server_fhandle *)fh->priv_fh;
  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    if (fh_aux != NULL) {
      //if(server_aux->params.sem_server != 0) sem_wait(server_aux->params.sem_server);
      ret = real_posix_close(fh_aux->fd);
      //if(server_aux->params.sem_server != 0) sem_post(server_aux->params.sem_server);
    }
  }
  /************** REMOTE ****************/
  else {
    //bzero(&msg, sizeof(struct st_sck_server_msg));

    msg.type = SCK_SERVER_CLOSE_FILE_WS;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    msg.u_st_sck_server_msg.op_close.fd = fh_aux->fd;
    memccpy(msg.u_st_sck_server_msg.op_close.path, fh_aux->path, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (ret), sizeof(int));
    debug_info("[NFI-SCK-CLI] nfi_sck_server_close(ID=%s): close->%d\n", server_aux->id, msg.u_st_sck_server_msg.op_close.fd);
  }

  // free memory
  FREE_AND_NULL(fh->priv_fh);
  fh->type = NFINULL;
  fh->server = NULL;

  DEBUG_END();

  // Return OK
  return ret;
}


int nfi_sck_server_remove(struct nfi_server * serv, char * url) {
  int ret;
  char server[PATH_MAX], dir[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct st_sck_server_msg msg;

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(url, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_remove(%s): begin %s\n", server_aux->id, url);
  if (server_aux == NULL) {
    sck_server_err(SCK_SERVER_ERR_PARAM);
    return -1;
  }

  // from url->server + dir
  ret = ParseURL(url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_remove: url %s incorrect.\n", url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    ret = real_posix_unlink(path);
    if (ret < 0) {
      debug_error("real_posix_open fails to open '%s' in server %s.\n", dir, serv->server);
      return -1;
    }
  }
  /************** REMOTE ****************/
  else {
    msg.type = SCK_SERVER_RM_FILE;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_rm.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (ret), sizeof(int));
  }

  DEBUG_END();

  return ret;
}



int nfi_sck_server_rename(struct nfi_server * serv, char * old_url, char * new_url) {
  int ret;
  char server[PATH_MAX], old_path[PATH_MAX], new_path[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(old_url, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(new_url, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_remove(%s): begin %s\n", server_aux->id, new_url);
  if (server_aux == NULL) {
    sck_server_err(SCK_SERVER_ERR_PARAM);
    return -1;
  }

  ret = ParseURL(old_url, NULL, NULL, NULL, server, NULL, old_path);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_open: url %s incorrect.\n", old_url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  ret = ParseURL(new_url, NULL, NULL, NULL, server, NULL, new_path);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_open: url %s incorrect.\n", new_url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char old_name [PATH_MAX];
    char new_name [PATH_MAX];

    strcpy(old_name, server_aux->params.dirbase);
    strcat(old_name, "/");
    strcat(old_name, old_path);

    strcpy(new_name, server_aux->params.dirbase);
    strcat(new_name, "/");
    strcat(new_name, new_path);

    ret = real_posix_rename(old_name, new_name);
    if (ret < 0) {
      debug_error("real_posix_rename fails to rename '%s' in server %s.\n", old_path, serv->server);
      return -1;
    }
  }
  /************** REMOTE ****************/
  else {
    msg.type = SCK_SERVER_RENAME_FILE;

    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_rename.old_url, old_path, 0, PATH_MAX - 1);
    memccpy(msg.u_st_sck_server_msg.op_rename.new_url, new_path, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (ret), sizeof(int));
  }

  DEBUG_END();

  return ret;
}



int nfi_sck_server_getattr(struct nfi_server * serv, struct nfi_fhandle * fh, struct nfi_attr * attr) {
  int ret;
  char server[PATH_MAX], dir[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  //struct nfi_sck_server_fhandle *fh_aux;
  struct st_sck_server_msg msg;
  struct st_sck_server_attr_req req;

  DEBUG_BEGIN();

  // check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(attr, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // copy private information...
  //fh_aux     = (struct nfi_sck_server_fhandle *) fh->priv_fh; //TODO: fstat
  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  ret = ParseURL(fh->url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_getattr: url %s incorrect.\n", dir);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    req.status = real_posix_stat(path, & (req.attr));
    if (((int) req.status) < 0) {
      debug_error("nfi_sck_server_getattr: Fail stat %s.\n", dir);
      return req.status;
    }
  }
  /************** REMOTE ****************/
  else {
    msg.type = SCK_SERVER_GETATTR_FILE;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_getattr.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& req, sizeof(struct st_sck_server_attr_req));
  }

  sck_server_2_nfi_attr(attr, & req.attr);

  DEBUG_END();

  // return status
  return req.status;
}



int nfi_sck_server_setattr(struct nfi_server * serv, struct nfi_fhandle * fh, struct nfi_attr * attr) {
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(attr, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh->priv_fh, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  fh_aux = (struct nfi_sck_server_fhandle *)fh->priv_fh;
  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  // TODO: setattr
  server_aux = server_aux;
  fh_aux = fh_aux;

  DEBUG_END();

  return 0;
}



int nfi_sck_server_mkdir(struct nfi_server * serv, char * url, struct nfi_attr * attr, struct nfi_fhandle * fh) {
  int ret;
  char server[PATH_MAX], dir[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;
  struct st_sck_server_attr_req req;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(attr, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  ret = ParseURL(url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_mkdir: url %s incorrect.\n", url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  // private_info file handle
  fh_aux = (struct nfi_sck_server_fhandle *)malloc(sizeof(struct nfi_sck_server_fhandle));
  NULL_RET_ERR(fh_aux, SCK_SERVER_ERR_MEMORY);
  bzero(fh_aux, sizeof(struct nfi_sck_server_fhandle));

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    ret = real_posix_mkdir(path, /*attr->at_mode*/ 0777);
    if ((ret < 0) && (errno != EEXIST)) {
      debug_error("nfi_sck_server_mkdir: Fail mkdir %s.\n", dir);
      FREE_AND_NULL(fh_aux);
      return -1;
    }
    fh_aux->fd = ret; //Cuidado

    //Get stat
    ret = real_posix_stat(dir, & (req.attr));
    if (ret < 0) {
      debug_error("nfi_sck_server_create: Fail stat %s.\n", dir);
      return ret;
    }
  }
  /************** SERVER ****************/
  else {
    msg.type = SCK_SERVER_MKDIR_DIR;
    memccpy(msg.u_st_sck_server_msg.op_mkdir.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (fh_aux->fd), sizeof(int));
    memccpy(fh_aux->path, dir, 0, PATH_MAX - 1);

    if ((fh_aux->fd < 0) && (errno != EEXIST)) {
      sck_server_err(SCK_SERVER_ERR_MKDIR);
      fprintf(stderr, "nfi_sck_server_mkdir: Fail mkdir %s in server %s.\n", dir, serv->server);
      FREE_AND_NULL(fh_aux);
      return -1;
    }

    //Get stat
    msg.type = SCK_SERVER_GETATTR_FILE;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_getattr.path, dir, 0, PATH_MAX - 1);

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& req, sizeof(struct st_sck_server_attr_req));
  }

  fh->type = NFIDIR;
  fh->priv_fh = (void *)fh_aux;

  fh->url = STRING_MISC_StrDup(url);
  if (fh->url == NULL) {
    sck_server_err(SCK_SERVER_ERR_MEMORY);
    FREE_AND_NULL(fh_aux);
    return -1;
  }

  sck_server_2_nfi_attr(attr, & req.attr);

  DEBUG_END();

  return ret;
}



int nfi_sck_server_opendir(struct nfi_server * serv, char * url, struct nfi_fhandle * fho) {
  int ret;
  char dir[PATH_MAX], server[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(url, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fho, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  ret = ParseURL(url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_opendir: url %s incorrect.\n", url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  fho->url = strdup(url);
  NULL_RET_ERR(fho->url, SCK_SERVER_ERR_MEMORY);

  fh_aux = (struct nfi_sck_server_fhandle *)malloc(sizeof(struct nfi_sck_server_fhandle));
  if (fh_aux == NULL) {
    sck_server_err(SCK_SERVER_ERR_MEMORY);
    FREE_AND_NULL(fho->url);
    return -1;
  }

  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    fh_aux->dir = real_posix_opendir(path);
    if (fh_aux->dir == NULL) {
      FREE_AND_NULL(fh_aux);
      FREE_AND_NULL(fho->url);
      debug_error("real_posix_opendir fails to open directory '%s' in server '%s'.\n", dir, serv->server);
      return -1;
    }
  }
  /************** SERVER ****************/
  else {
    msg.type = SCK_SERVER_OPENDIR_DIR;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    memccpy(msg.u_st_sck_server_msg.op_opendir.path, dir, 0, PATH_MAX - 1);

    unsigned long long aux;
    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (aux), sizeof(DIR * ));

    fh_aux->dir = (DIR *)aux;
  }

  strcpy(fh_aux->path, dir);
  fho->type = NFIDIR;
  fho->server = serv;
  fho->priv_fh = (void *)fh_aux;

  DEBUG_END();

  return 0;
}



int nfi_sck_server_readdir(struct nfi_server * serv, struct nfi_fhandle * fh, struct dirent * entry) {
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;
  struct st_sck_server_msg msg;
  struct st_sck_server_direntry ret_entry;
  struct dirent * ent;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh->priv_fh, SCK_SERVER_ERR_PARAM);

  if (fh->type != NFIDIR) {
    sck_server_err(SCK_SERVER_ERR_NOTDIR);
    return -1;
  }

  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  fh_aux = (struct nfi_sck_server_fhandle *)fh->priv_fh;

  // clean all entry content
  memset(entry, 0, sizeof(struct dirent));

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    ent = real_posix_readdir(fh_aux->dir);
    if (ent == NULL) {
      debug_error("nfi_sck_server_readdir: readdir");
      return -1;
    }

    memcpy(entry, ent, sizeof(struct dirent));
  }
  /************** SERVER ****************/
  else {
    msg.type = SCK_SERVER_READDIR_DIR;
    memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
    msg.u_st_sck_server_msg.op_readdir.dir = fh_aux->dir;

    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (ret_entry), sizeof(struct st_sck_server_direntry)); //NEW

    if (ret_entry.end == 0) {
      return -1;
    }

    memcpy(entry, & (ret_entry.ret), sizeof(struct dirent));
  }

  DEBUG_END();

  return 0;
}



int nfi_sck_server_closedir(struct nfi_server * serv, struct nfi_fhandle * fh) {
  int ret;
  struct st_sck_server_msg msg;
  struct nfi_sck_server_server * server_aux;
  struct nfi_sck_server_fhandle * fh_aux;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(fh, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // do closedir...
  if (fh->priv_fh != NULL) {
    // private_info...
    server_aux = (struct nfi_sck_server_server *)serv->private_info;
    fh_aux = (struct nfi_sck_server_fhandle *)fh->priv_fh;

    /************** LOCAL *****************/
    if (server_aux->params.locality) {
      real_posix_closedir(fh_aux->dir);
    }
    /************** SERVER ****************/
    else {
      msg.type = SCK_SERVER_CLOSEDIR_DIR;

      memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
      msg.u_st_sck_server_msg.op_closedir.dir = fh_aux->dir;

      nfi_sck_server_doRequest(server_aux, & msg, (char *)& (ret), sizeof(int)); //NEW

    }

    // free memory
    FREE_AND_NULL(fh->priv_fh);
  }

  DEBUG_END();

  return 0;
}



int nfi_sck_server_rmdir(struct nfi_server * serv, char * url) {
  int ret;
  char server[PATH_MAX], dir[PATH_MAX];
  struct nfi_sck_server_server * server_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(url, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  ret = ParseURL(url, NULL, NULL, NULL, server, NULL, dir);
  if (ret < 0) {
    fprintf(stderr, "nfi_sck_server_rmdir: url %s incorrect.\n", url);
    sck_server_err(SCK_SERVER_ERR_URL);
    return -1;
  }

  /************** LOCAL *****************/
  if (server_aux->params.locality) {
    char path [PATH_MAX];

    strcpy(path, server_aux->params.dirbase);
    strcat(path, "/");
    strcat(path, dir);

    ret = real_posix_rmdir(path);
    if (ret < 0) {
      debug_error(stderr, "nfi_sck_server_rmdir: Fail rmdir %s.\n", dir);
      return -1;
    }
  }
  /************** SERVER ****************/
  else {
    msg.type = SCK_SERVER_RMDIR_DIR;
    debug_info("RMDIR - %d\n", msg.type);
    memccpy(msg.u_st_sck_server_msg.op_rmdir.path, dir, 0, PATH_MAX - 1);
    nfi_sck_server_doRequest(server_aux, & msg, (char *)& (ret), sizeof(int));

    if (ret < 0) {
      fprintf(stderr, "nfi_sck_server_rmdir: Fail rmdir %s in server %s.\n", dir, serv->server);
      sck_server_err(SCK_SERVER_ERR_REMOVE);
      return -1;
    }
  }

  DEBUG_END();

  return 0;
}



int nfi_sck_server_statfs(__attribute__((__unused__)) struct nfi_server * serv, __attribute__((__unused__)) struct nfi_info * inf) {
  DEBUG_BEGIN();

  /*
  struct sck_server_info sck_serverinf;
  int ret;
  struct nfi_sck_server_server *server_aux;

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM) ;
  NULL_RET_ERR(inf,  SCK_SERVER_ERR_PARAM) ;
  nfi_sck_server_keepConnected(serv) ;
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM) ;

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;

  ret = sck_server_statfs(server_aux->fh, &sck_serverinf, server_aux->cl) ;
  if (ret <0).{
    sck_server_err(SCK_SERVER_ERR_STATFS) ;
    return -1;
  }

  SCKtoNFIInfo(inf, &sck_serverinf) ;
  */

  //TODO

  DEBUG_END();

  return 0;
}



int nfi_sck_server_preload(struct nfi_server * serv, char * url, char * virtual_path, char * storage_path, int opt) {
  //char dir[PATH_MAX];
  int ret;
  struct nfi_sck_server_server * server_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(url, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(virtual_path, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(storage_path, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // get private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_preload(ID=%s): begin %s - %s \n", server_aux->id, virtual_path, storage_path);

  if (url[strlen(url) - 1] == '/') {
    sck_server_err(SCK_SERVER_ERR_PARAM);
    return -1;
  }

  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_preload(ID=%s): preload %s in server %s.\n", server_aux->id, virtual_path, serv->server);

  /*****************************************/
  msg.type = SCK_SERVER_PRELOAD_FILE;
  memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
  memccpy(msg.u_st_sck_server_msg.op_preload.virtual_path, virtual_path, 0, PATH_MAX - 1);
  memccpy(msg.u_st_sck_server_msg.op_preload.storage_path, storage_path, 0, PATH_MAX - 1);
  msg.u_st_sck_server_msg.op_preload.block_size = serv->block_size;
  msg.u_st_sck_server_msg.op_preload.opt = opt;

  nfi_sck_server_doRequest(server_aux, & msg, (char *)& ret, sizeof(int));
  /*****************************************/

  debug_info("[NFI-SCK-CLI] nfi_sck_server_preload(ID=%s): end %s - %s = %d\n", server_aux->id, virtual_path, storage_path, ret);

  DEBUG_END();

  return ret;
}



int nfi_sck_server_flush(struct nfi_server * serv, char * url, char * virtual_path, char * storage_path, int opt) {
  //char dir[PATH_MAX];
  int ret;
  struct nfi_sck_server_server * server_aux;
  struct st_sck_server_msg msg;

  DEBUG_BEGIN();

  // Check arguments...
  NULL_RET_ERR(serv, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(url, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(virtual_path, SCK_SERVER_ERR_PARAM);
  NULL_RET_ERR(storage_path, SCK_SERVER_ERR_PARAM);
  nfi_sck_server_keepConnected(serv);
  NULL_RET_ERR(serv->private_info, SCK_SERVER_ERR_PARAM);

  // private_info...
  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_flush(ID=%s): begin %s - %s \n", server_aux->id, virtual_path, storage_path);

  if (url[strlen(url) - 1] == '/') {
    sck_server_err(SCK_SERVER_ERR_PARAM);
    return -1;
  }

  server_aux = (struct nfi_sck_server_server *)serv->private_info;
  debug_info("[NFI-SCK-CLI] nfi_sck_server_flush(ID=%s): open %s in server %s.\n", server_aux->id, virtual_path, serv->server);

  /*****************************************/
  //bzero(&msg, sizeof(struct st_sck_server_msg));

  msg.type = SCK_SERVER_FLUSH_FILE;
  memccpy(msg.id, server_aux->id, 0, SCK_SERVER_ID - 1);
  memccpy(msg.u_st_sck_server_msg.op_flush.virtual_path, virtual_path, 0, PATH_MAX - 1);
  memccpy(msg.u_st_sck_server_msg.op_flush.storage_path, storage_path, 0, PATH_MAX - 1);
  msg.u_st_sck_server_msg.op_flush.block_size = serv->block_size;
  msg.u_st_sck_server_msg.op_flush.opt = opt;

  nfi_sck_server_doRequest(server_aux, & msg, (char *)& ret, sizeof(int));
  /*****************************************/

  DEBUG_END();

  return 0;
}

/* ................................................................... */
