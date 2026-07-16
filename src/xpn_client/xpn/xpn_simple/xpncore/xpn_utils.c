
/*
 *  Copyright 2000-2026 The Expand Team.
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

     #include "xpn/xpn_simple/xpn_utils.h"


  /* ... Functions / Funciones ......................................... */


     int xpn_simple_preload ( const char * virtual_path, const char * storage_path )
     {
         char abs_path[PATH_MAX], url_serv[PATH_MAX];
         struct nfi_server * servers;
         struct xpn_metadata * mdata;
         int res, n, pd;

         // Check arguments
         if (virtual_path == NULL)
         {
             errno = ENOENT;
             return -1;
         }

         if (storage_path == NULL)
         {
             errno = ENOENT;
             return -1;
         }

         // Create file in the expand partition
         res = XpnGetAbsolutePath(virtual_path, abs_path); // this function generates the absolute path
         if (res < 0)
         {
             errno = ENOENT;
             return res;
         }

         pd = XpnGetPartition(abs_path); // returns partition id and remove partition name from abs_path
         if (pd < 0)
         {
             errno = ENOENT;
             return pd;
         }

         servers = NULL;
         n = XpnGetServers(pd, -1, &servers);
         if (n <= 0)
         {
             return res;
         }

         // Create metadata 
         mdata = (struct xpn_metadata * ) malloc(sizeof(struct xpn_metadata));
         if (mdata == NULL)
         {
             return -1;
         }

         memset(mdata, 0, sizeof(*mdata));

         XpnCreateMetadata(mdata, pd, abs_path);

         // (1/2) Request...
         for (int j = 0; j < n; ++j)
         {
             XpnGetURLServer(&servers[j], abs_path, url_serv);

             // Worker
             servers[j].wrk->thread = servers[j].xpn_thread;
             nfi_worker_do_preload(servers[j].wrk, url_serv, (char * ) url_serv, (char * )storage_path, XpnSearchPart(pd)->block_size, XpnSearchPart(pd)->replication_level);
         }

         // (2/2) ... and Wait
         for (int j = 0; j < n; ++j)
         {
             res = nfiworker_wait(servers[j].wrk);
         }

         if (servers != NULL)
         {
             free(servers);
         }

         if (mdata != NULL)
         {
             free(mdata);
         }

         // error checking
         if (res)
         {
             errno = ENOENT;
             return -1;
         }

         return 0;
     }


     int xpn_simple_flush ( const char * virtual_path, const char * storage_path )
     {
         char abs_path[PATH_MAX], url_serv[PATH_MAX];
         struct nfi_server * servers;
         struct xpn_metadata * mdata;
         int res, n, pd;

         // Check arguments
         if (virtual_path == NULL)
         {
             errno = ENOENT;
             return -1;
         }

         if (storage_path == NULL)
         {
             errno = ENOENT;
             return -1;
         }

         // Open file in the expand partition
         res = XpnGetAbsolutePath(virtual_path, abs_path); // this function generates the absolute path
         if (res < 0)
         {
             errno = ENOENT;
             return res;
         }

         pd = XpnGetPartition(abs_path); // returns partition id and remove partition name from abs_path
         if (pd < 0)
         {
             errno = ENOENT;
             return pd;
         }

         servers = NULL;
         n = XpnGetServers(pd, -1, &servers);
         if (n <= 0)
         {
             return res;
         }

         // Read metadata 
         mdata = (struct xpn_metadata * ) malloc(sizeof(struct xpn_metadata));
         if (mdata == NULL)
         {
             return -1;
         }

         memset(mdata, 0, sizeof(*mdata));

         res = XpnReadMetadata(mdata, n, servers, abs_path, XpnSearchPart(pd)->replication_level);
         if (res < 0)
         {
             free(mdata);
             return -1;
         }

         // (1/2) Request...
         for (int j = 0; j < n; ++j)
         {
             XpnGetURLServer(&servers[j], abs_path, url_serv);

             // Worker
             servers[j].wrk->thread = servers[j].xpn_thread;
             nfi_worker_do_flush(servers[j].wrk, url_serv, (char * ) url_serv, (char * )storage_path, XpnSearchPart(pd)->block_size, XpnSearchPart(pd)->replication_level);
         }

         // (2/2) ... and Wait
         for (int j = 0; j < n; ++j)
         {
             res = nfiworker_wait(servers[j].wrk);
         }

         if (servers != NULL)
         {
             free(servers);
         }

         if (mdata != NULL)
         {
             free(mdata);
         }

         // error checking
         if (res)
         {
             errno = ENOENT;
             return -1;
         }

         return 0;
     }


  /* ................................................................... */
