![Logo for the Expand Parallel File System.](info/xpn-logo.svg)

# Ad-Hoc XPN 3.3.1

*Expand Ad-Hoc Parallel File System*

[![License: GPL3](https://img.shields.io/badge/License-GPL3-blue.svg)](https://opensource.org/licenses/GPL-3.0)
![version](https://img.shields.io/badge/version-3.3.1-blue)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ca0c40db97f64698a2db9992cafdd4ab)](https://app.codacy.com/gh/xpn-arcos/xpn/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

* *Homepage*:     <https://xpn-arcos.github.io/xpn-arcos.github.io/>
* *Source*:       <https://github.com/xpn-arcos/xpn>
* *Licence*:      [GNU GENERAL PUBLIC LICENSE Version 3](https://github.com/dcamarmas/xpn/blob/master/COPYING)</br>
* *Maintainers*:  Felix Garcia Carballeira, Luis Miguel Sanchez Garcia, Borja Bergua Guerra, Alejandro Calderon Mateos, Diego Camarmas Alonso,  Elias Del Pozo Puñal, Dario Muñoz Muñoz, Gabriel Sotodosos Morales


## Contents

  * [1. To deploy Ad-Hoc XPN...](#1-to-deploy-ad-hoc-xpn)
    * [1.1 Deploying on a cluster/supercomputer](#11-deploying-ad-hoc-expand-on-a-clustersupercomputer)
    * [1.2 Deploying on a IoT distributed system](#12-deploying-ad-hoc-expand-on-a-iot-distributed-system)
  * [2. Executing Ad-Hoc XPN...](#2-executing-ad-hoc-xpn)
    * [2.1 Executing Ad-Hoc Expand using MPICH](#21-executing-ad-hoc-expand-using-mpich)
    * [2.2 Executing Ad-Hoc Expand using OpenMPI (experimental)](#22-executing-ad-hoc-expand-using-openmpi-experimental-alpha)
  * [Publications](#publications)


## 1. To deploy Ad-Hoc XPN...

### 1.1 Deploying Ad-Hoc Expand on a cluster/supercomputer

  The Expand Ad-Hoc Parallel File System (a.k.a. Ad-Hoc XPN) can be installed on a cluster/supercomputer with:
  * A local storage per-node (HDD, SSD or RAM Drive) accessible through a directory, for example, ```/tmp```. <br>
    This will be the NODE_DIR in this document.
  * A shared directory among compute nodes used, for example ```$HOME```. <br>
    This will be the WORK_DIR in this document.

  There are only two software pre-requisites that Ad-Hoc XPN needs:
  1. The typical C development tools: gcc, make, and autotools
  2. An MPI implementation compiled with MPI-IO and threads support:
     * Tested: MPICH 4.1.1 (or compatible) and Intel MPI 2017 (or compatible).
     * Experimental: OpenMPI 5.0.3 (support is experimental).

     If you do not have a compiled MPI implementation with MPI-IO and thread support, <br> still you can compile MPICH or OpenMPI (experimental) from source code:
     * <details>
       <summary>Help to compile MPICH from source code... (click to expand)</summary>
        <br>
        In order to install the MPICH implementation of MPI (for example, MPICH 4.1.1) from source code and with Infiniband (Omni-Path) support we recommend:

        ```
        wget https://www.mpich.org/static/downloads/4.1.1/mpich-4.1.1.tar.gz
        tar zxf mpich-4.1.1.tar.gz

        cd mpich-4.1.1
        ./configure --prefix=<path where MPICH is going to be installed> \
                    --enable-threads=multiple \
                    --enable-romio \
                    --with-slurm=<path where your slurm is installed> \
                    --with-device=ch4:ofi:psm2 \
                    --with-libfabric=<path where your libfabric is installed>

        make
        make install

        export LD_LIBRARY_PATH=<path where MPICH is going to be installed>/lib:$LD_LIBRARY_PATH
        ```
       </details>
     * <details>
        <summary>Help to compile OpenMPI from source code... (click to expand)</summary>
        <br>
        For example, in order to install the OpenMPI 5.0.3 implementation of MPI from source code, including Infiniband (Omni-Path) support, we recommend the following steps:

        ```
        wget https://download.open-mpi.org/release/open-mpi/v5.0/openmpi-5.0.3.tar.gz
        tar zxf openmpi-5.0.3.tar.gz

        cd openmpi-5.0.3
        ./configure --prefix=<path where Open MPI is going to be installed> \
                    --enable-threads=multiple \
                    --enable-romio \
                    --with-slurm=<path where your slurm is installed> \
                    --with-libfabric=<path where your libfabric is installed>

        make
        make install

        export LD_LIBRARY_PATH=<path where Open MPI is going to be installed>/lib:$LD_LIBRARY_PATH
        ```
       </details>


  The general steps to deploy XPN are:
  ```mermaid
  %% {init: {"flowchart": {"defaultRenderer": "elk"}} }%%
  flowchart TD
    A([Start]) --> B("Do you have 'Spack'?")
    B -- Yes --> ide11
    B -- No --> Y1("Do you have 'modules'?")

    %% (1) with spack
    subgraph ide1 [1 With spack]
    subgraph ide11 [1.1 Add repo]
       direction TB
       X1["git clone https#58;#47;#47;github.com/xpn-arcos/xpn.git
          spack repo add xpn/scripts/spack"]
    end
    subgraph ide12 [1.2 Install software]
       direction TB
       X2["spack <b>info</b> xpn
          spack <b>install</b> xpn"]
    end
    subgraph ide13 [1.3 Load software]
       direction TB
       X3["spack <b>load</b> xpn"]
    end
    classDef lt text-align:left,fill:lightgreen,color:black;
    class X1,X2,X3 lt;
    ide11 --> ide12
    ide12 --> ide13
    end
    ide13 --> I([End])

    Y1-- Yes --> ide21a
    Y1-- No ---> ide21b
    subgraph ide2 [2 With autotools]
    subgraph ide21a [2.1 Load prerequisites]
       direction TB
       Y1A["module avail <br> module load gcc<br> module load 'impi/2017.4'"]
    end
    subgraph ide21b [2.1 Install prerequisites]
       direction TB
       Y1B["sudo apt install -y build-essential libtool autoconf automake git<br><br>sudo apt install -y libmpich-dev mpich"]
    end
    subgraph ide22 [2.2 Download source code]
       direction TB
       Y2B["mkdir $HOME/src
            cd    $HOME/src
            git clone ‎https\://github.com/xpn-arcos/xpn.git"]
    end
    subgraph ide23 ["2.3 Build source code"]
       direction LR
       Y3B["export XPN_MPICC='full path to the mpicc compiler to be used'
            cd $HOME/src
            ./xpn/build-me -m $XPN_MPICC -i $HOME/bin"]
    end
    ide21a --> ide22
    ide21b --> ide22
    ide22 --> ide23

    classDef lt2 text-align:left,fill:lightblue,color:black;
    class Y1A,Y1B lt2;
    classDef lt3 text-align:left;
    class Y2B,Y3B lt3;
    end

    Y3B --> I([End])
  ```


### 1.2 Deploying Ad-Hoc Expand on a IoT distributed system

  The Expand Ad-Hoc Parallel File System (a.k.a. Ad-Hoc XPN) can be installed on a IoT distributed system with:
  * A local storage per-node (HDD, SSD or RAM Drive) accessible through a directory, for example, ```/tmp```. <br>
    This will be the NODE_DIR in this document.

  There are only two software pre-requisites that Ad-Hoc XPN for IoT needs:
  1. The typical C development tools: gcc, make, and autotools
  2. A MQTT implementation:
     * Tested: Mosquitto 2.0.18 (or compatible)

  To install the typical prerequisites, the general steps are:
  * Install the typical C development tools:
    ```
    sudo apt install -y build-essential libtool autoconf automake git
    ```
  * In order to use Mosquitto, you must install the following packages:
    ```
    sudo apt-get install mosquitto mosquitto-clients mosquitto-dev libmosquitto-dev
    ```
  * Then, we need to stop the Mosquitto MQTT service:
    ```
    sudo systemctl stop mosquitto
    ```

  Once all prerequisites are met, the general steps to deploy XPN are:
  * Download XPN source code:
    ```
    mkdir $HOME/src
    cd    $HOME/src
    git clone https://github.com/xpn-arcos/xpn.git
    ```
  * Build source code
    ```
    export XPN_CC='full path to the gcc compiler to be used'
    export MQTT_PATH='/usr/lib/x86_64-linux-gnu/libmosquitto.so'
    cd $HOME/src
    ./xpn/scripts/compile/software/xpn_iot_mpi.sh -m $XPN_CC -i $HOME/bin -q $MQTT_PATH -s ./xpn
    ```


## 2. Executing Ad-Hoc XPN...

First, you need to get familiar with 2 special files and 1 special environment variables for XPN client:

  ```mermaid
  %%{ init : { "theme" : "default", "themeVariables" : { "background" : "#000" }}}%%
  mindmap
  root(("Ad-Hoc XPN"))
    {{Environment<br> Variables}}
        ["`**XPN_CONF=**´full path to the xpn.conf file´ <br> \* It is the XPN configuration file to be used (mandatory)`"]
    {{Files}}
        ["`**hostfile**</br>   \* for MPI, it is a text file with the list of host names (one per line) where XPN servers and XPN client is going to be executed`"]
        ["`**xpn.conf**</br>   \* for Ad-Hoc XPN, it is a text file with the configuration for the partition where files are stored at the XPN servers`"]
```


<details>
<summary>For Expand developers...</summary>
You need to get familiar with 3 special files and 4 special environment variables for XPN client:

  ```mermaid
  %%{ init : { "theme" : "default", "themeVariables" : { "background" : "#000" }}}%%
  mindmap
  root((Ad-Hoc XPN))
    {{Files}}
        [hostfile]
        [xpn.cfg]
        [stop_file]
    {{Environment Variables}}
        [XPN_CONF]
        [XPN_THREAD]
        [XPN_LOCALITY]
        [XPN_SCK_PORT]
        [XPN_SCK_IPV]
        [XPN_CONNECTED]
        [XPN_MQTT]
        [XPN_MQTT_QOS]
```

The 3 special files are:
* ```<hostfile>``` for MPI, it is a text file with the list of host names (one per line) where XPN servers and XPN client is going to be executed.
* ```<xpn.cfg>``` for XPN, it is the XPN configuration file with the configuration for the partition where files are stored at the XPN servers.
* ```<stop_file>``` for XPN is a text file with the list of the servers to be stopped (one host name per line).

And the 8 special environment variables for XPN clients are:
* ```XPN_CONF```       with the full path to the XPN configuration file to be used (mandatory).
* ```XPN_THREAD```     with value 0 for without threads, value 1 for thread-on-demand and value 2 for pool-of-threads (optional, default: 0).
* ```XPN_LOCALITY```   with value 0 for without locality and value 1 for with locality (optional, default: 1).
* ```XPN_SCK_PORT```   with the port to use in internal comunications (opcional, default: 3456).
* ```XPN_SCK_IPV```    with value 6 for IPv6 support or value 4 for IPv4 support (optional, default: 4).
* ```XPN_CONNECTED```  with value 0 for connection per request or value 1 for connection per session (optional, default: 1).
* ```XPN_MQTT```       with value 1 for MQTT support (optional, default: 0).
* ```XPN_MQTT_QOS```   with value 0, 1, 2 for the QoS of MQTT (optional, default: 0).
</details>


### 2.1 Executing Ad-Hoc Expand using MPICH

An example of SLURM job might be:
   ```bash
   #!/bin/bash

   #SBATCH --job-name=test
   #SBATCH --output=$HOME/results_%j.out
   #SBATCH --nodes=8
   #SBATCH --ntasks=8
   #SBATCH --cpus-per-task=8
   #SBATCH --time=00:05:00

   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
   export NODE_DIR=<local directory to be used on each node, /tmp for example>

   scontrol show hostnames ${SLURM_JOB_NODELIST} > $WORK_DIR/hostfile

   # Step 1: to launch the Expand MPI servers
   <INSTALL_PATH>/xpn/bin/xpn -v \
                              -w $WORK_DIR -x $NODE_DIR \
                              -n <number of XPN processes> \
                              -l $WORK_DIR/hostfile start
   sleep 2

   # Step 2: to launch the XPN client (app. that will use Expand)
   mpiexec -np <number of client processes> \
           -hostfile $WORK_DIR/hostfile \
           -genv XPN_CONF    $WORK_DIR/xpn.conf \
           -genv LD_PRELOAD  <INSTALL_PATH>/xpn/lib/xpn_bypass.so:$LD_PRELOAD \
           <full path to the app.>

   # Step 3: to stop the MPI servers
   <INSTALL_PATH>/xpn/bin/xpn -v -d $WORK_DIR/hostfile stop
   sleep 2
   ```


The typical executions has 3 main steps:
1. First, launch the Expand MPI servers:
   ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
   export NODE_DIR=<local directory to be used on each node, /tmp for example>

   <INSTALL_PATH>/xpn/bin/xpn -v \
                              -n <number of processes> \
                              -l $WORK_DIR/hostfile \
                              -w $WORK_DIR \
                              -x $NODE_DIR \
                              start
   ```
2. Then, launch the program that will use Expand (XPN client).

   2.1. Example for the *app1* MPI application:
      ```bash
        export WORK_DIR=<shared directory among hostfile computers, $HOME for example>

        mpiexec -np               <number of processes> \
                -hostfile         $WORK_DIR/hostfile \
                -genv XPN_CONF    $WORK_DIR/xpn.conf \
                -genv LD_PRELOAD  <INSTALL_PATH>/xpn/lib/xpn_bypass.so:$LD_PRELOAD \
                <full path to app1>/app1
      ```
   2.2. Example for the *app2* program (a NON-MPI application):
      ```bash
        export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
        export XPN_CONF=$WORK_DIR/xpn.conf

        LD_PRELOAD=<INSTALL_PATH>/xpn/lib/xpn_bypass.so <full path to app2>/app2
      ```
   2.3. Example for the *app3.py* Python program:
      ```bash
        export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
        export XPN_CONF=$WORK_DIR/xpn.conf

        LD_PRELOAD=<INSTALL_PATH>/xpn/lib/xpn_bypass.so python3 <full path to app3>/app3.py
      ```
4. At the end of your working session, you need to stop the MPI servers:
   ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>

   <INSTALL_PATH>/xpn/bin/xpn  -v -d $WORK_DIR/hostfile stop
   ```


### 2.2 Executing Ad-Hoc Expand using OpenMPI (experimental alpha)

An example of SLURM job might be:
   ```bash
   #!/bin/bash

   #SBATCH --job-name=test
   #SBATCH --output=$HOME/results_%j.out
   #SBATCH --nodes=8
   #SBATCH --exclusive
   #SBATCH --time=00:05:00

   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
   export NODE_DIR=<local directory to be used on each node, /tmp for example>

   scontrol show hostnames ${SLURM_JOB_NODELIST} > $WORK_DIR/hostfile

   <INSTALL_PATH>/scripts/execute/xpn.sh -w $WORK_DIR -l $WORK_DIR/hostfile -x $NODE_DIR -n ${SLURM_NNODES} -v mk_conf

   # Step 1
   prte --hostfile $WORK_DIR/hostfile --report-uri $WORK_DIR/prte --no-ready-msg &
   sleep 2
   NAMESPACE=$(cat $WORK_DIR/prte | head -n 1 | cut -d "@" -f 1)

   # Step 2: to launch the Expand MPI servers
   mpiexec -n ${SLURM_NNODES} \
           --hostfile $WORK_DIR/hostfile \
           --dvm ns:$NAMESPACE \
           --map-by ppr:1:node:OVERSUBSCRIBE \
           <INSTALL_PATH>/bin/xpn_server &
   sleep 2

   # Step 3: to launch the XPN client (app. that will use Expand)
   mpiexec  -n <number of processes: 2> \
            -hostfile $WORK_DIR/hostfile \
            --dvm ns:$NAMESPACE \
            -mca routed direct \
            --map-by node:OVERSUBSCRIBE \
            -x XPN_CONF=$WORK_DIR/xpn.conf \
            -x LD_PRELOAD=<INSTALL_PATH>/xpn/lib/xpn_bypass.so:$LD_PRELOAD \
            <full path to the app.>

   # Step 4: to stop the MPI servers
   <INSTALL_PATH>/xpn/bin/xpn -v -d $WORK_DIR/hostfile stop
   sleep 2
   ```


The typical executions has 4 main steps:
1. First, launch the OpenMPI prte server:
  ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
   export NODE_DIR=<local directory to be used on each node, /tmp for example>

   prte --hostfile $WORK_DIR/hostfile --report-uri $WORK_DIR/prte --no-ready-msg &
   NAMESPACE=$(cat $WORK_DIR/prte | head -n 1 | cut -d "@" -f 1)
   ```

2. Second, launch the Expand servers:
   ```bash
   mpiexec -n <number of processes>  -hostfile $WORK_DIR/hostfile \
           --dvm ns:$NAMESPACE \
           --map-by ppr:1:node:OVERSUBSCRIBE \
           <INSTALL_PATH>/bin/xpn_server &
   ```
3. Then, launch the program that will use Expand (XPN client).

   3.1. Example for the *app1* MPI application:
   ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>

   mpiexec -n <number of processes>  -hostfile $WORK_DIR/hostfile \
           -mca routed direct \
           --map-by node:OVERSUBSCRIBE \
           --dvm ns:$NAMESPACE \
           -genv XPN_CONF    $WORK_DIR/xpn.conf \
           -genv LD_PRELOAD  <INSTALL_PATH>/xpn/lib/xpn_bypass.so:$LD_PRELOAD \
           <full path to app1>/app1
   ```
   3.2. Example for the *app2* program (a NON-MPI application):
   ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
   export XPN_CONF=$WORK_DIR/xpn.conf

   LD_PRELOAD=<INSTALL_PATH>/xpn/lib/xpn_bypass.so <full path to app2>/app2
   ```
   3.3. Example for the *app3.py* Python program:
   ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>
   export XPN_CONF=$WORK_DIR/xpn.conf

   LD_PRELOAD=<INSTALL_PATH>/xpn/lib/xpn_bypass.so python3 <full path to app3>/app3.py
   ```
5. At the end of your working session, you need to stop the Expand servers:
   ```bash
   export WORK_DIR=<shared directory among hostfile computers, $HOME for example>

   ./xpn -v -d $WORK_DIR/hostfile stop
   ```


<details>
<summary>For Expand developers...</summary>
<br>
Summary:

```mermaid
sequenceDiagram
    job            ->> mk_conf.sh: generate the XPN configuration file
    mk_conf.sh     ->> xpn.conf: generate the xpn.conf file
    job            ->> xpn_server: (1) launch the Expand MPI server
    xpn.conf      -->> xpn_server: read the XPN configuration file
    job            ->> XPN client: (2) launch the program that will use Expand
    xpn.conf      -->> XPN client: read the XPN configuration file
    XPN client      -> xpn_server: write and read data
    XPN client    -->> job: execution ends
    job            ->> xpn_server: (3) stop the MPI server
```
</details>

## Publications

### 2025

<details>
<summary>:newspaper: Malleability and fault tolerance in ad-hoc parallel file systems</summary>
 
  * Authors: Dario Muñoz-Muñoz, Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos, Jesus Carretero
  * Journal paper: Cluster Computing
  * Link: [:link: Open publication](https://doi.org/10.1007/s10586-025-05575-8)
  * Cite:
  ```bash
  ```
</details>

<details>
<summary>:newspaper: Hierarchical and distributed data storage for Computing Continuum</summary>
 
  * Authors: Elias Del-Pozo-Puñal, Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos
  * Journal paper: Future Generation Computer Systems
  * Link: [:link: Open publication](https://doi.org/10.1016/j.future.2025.107931)
  * Cite:
  ```bash
@article{DELPOZOPUNAL2026107931,
title = {Hierarchical and distributed data storage for computing continuum},
journal = {Future Generation Computer Systems},
volume = {174},
pages = {107931},
year = {2026},
issn = {0167-739X},
doi = {https://doi.org/10.1016/j.future.2025.107931},
url = {https://www.sciencedirect.com/science/article/pii/S0167739X25002262},
author = {Elias Del-Pozo-Puñal and Felix Garcia-Carballeira and Diego Camarmas-Alonso and Alejandro Calderon-Mateos}}
  ```
</details>

<details>
<summary>:newspaper: Evaluación del sistema de ficheros Expand Ad-Hoc con aplicaciones de uso intensivo de datos</summary>
 
  * Authors: Diego Camarmas-Alonso, Felix Garcia-Carballeira, Alejandro Calderon-Mateos, Darío Muñoz-Muñoz, Jesus Carretero
  * Conference paper: XXXV Jornadas de Paralelismo (JP25)
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.15773123)
  * Cite:
  ```bash
@article{DIEGOCAMARMASALONSO_JP25,
   title = {Evaluación del sistema de ficheros Expand Ad-Hoc con aplicaciones de uso intensivo de datos},
   conference = {XXXV Jornadas de Paralelismo (JP25)},
   volume = {1},
   pages = {305-314},
   year = {2025},
   doi = {https://doi.org/10.5281/zenodo.15773123},
   url = {https://zenodo.org/records/15773123},
   author = {Diego Camarmas-Alonso, Felix Garcia-Carballeira, Alejandro Calderon-Mateos, Darío Muñoz-Muñoz, Jesus Carretero}}
  ```
</details>

<details>
<summary>:newspaper: Sistema de almacenamiento para computing continuum: aplicación a sistemas de información ferroviaria</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos
  * Conference paper: XXXV Jornadas de Paralelismo (JP25)
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.15773179)
  * Cite:
  ```bash
@article{DELPOZOPUNAL_JP25,
   title = {Sistema de almacenamiento para computing continuum: aplicación a sistemas de información ferroviaria},
   conference = {XXXV Jornadas de Paralelismo (JP25)},
   volume = {1},
   pages = {315-324},
   year = {2025},
   doi = {https://doi.org/10.5281/zenodo.15773179},
   url = {https://zenodo.org/records/15773179},
   author = {Elías Del-Pozo-Puñal, Félix García-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos}}
  ```
</details>

<details>
<summary>:newspaper: LFI: una librería de comunicaciones tolerante a fallos para redes de alto rendimiento</summary>
 
  * Authors: Darío Muñoz-Muñoz, Félix García-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos
  * Conference paper:XXXV Jornadas de Paralelismo (JP25)
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.15773016)
  * Cite:
  ```bash
@article{DARIOMUNOZMUNOZ_JP25,
   title = {LFI: una librería de comunicaciones tolerante a fallos para redes de alto rendimiento},
   conference = {XXXV Jornadas de Paralelismo (JP25)},
   volume = {1},
   pages = {205-214},
   year = {2025},
   doi = {https://doi.org/10.5281/zenodo.15773016},
   url = {https://zenodo.org/records/15773016},
   author = {Darío Muñoz-Muñoz, Félix García-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos}}
  ```
</details>

<details>
<summary>:newspaper: Optimización de entornos de Big Data Analytics mediante sistemas de ficheros paralelos Ad-hoc</summary>
 
  * Authors: Gabriel Sotodosos-Morales, Félix García-Carballeira, Diego Camarmas-Alonso, Alejandro Calderón-Mateos, Darío Muñoz-Muñoz, Jesús Carretero
  * Conference paper: XXXV Jornadas de Paralelismo (JP25)
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.15773242)
  * Cite:
  ```bash
@article{SOTODOSOSMORALES_JP25,
   title = {Optimización de entornos de Big Data Analytics mediante sistemas de ficheros paralelos Ad-hoc},
   conference = {XXXV Jornadas de Paralelismo (JP25)},
   volume = {1},
   pages = {833-842},
   year = {2025},
   doi = {https://doi.org/10.5281/zenodo.15773242},
   url = {https://zenodo.org/records/15773242},
   author = {Gabriel Sotodosos-Morales, Félix García-Carballeira, Diego Camarmas-Alonso, Alejandro Calderón-Mateos, Darío Muñoz-Muñoz, Jesús Carretero}}
  ```
</details>


### 2024

<details>
<summary>:newspaper: Fault tolerant in the Expand Ad-Hoc parallel file system</summary>
 
  * Authors: Dario Muñoz-Muñoz, Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos, Jesus Carretero
  * Conference paper: 30th International European Conference on Parallel and Distributed Computing (Euro-Par)
  * Link: [:link: Open publication](https://doi.org/10.1007/978-3-031-69766-1_5)
  * Cite:
  ```bash
@InProceedings{10.1007/978-3-031-69766-1_5,
   author="Mu{\~{n}}oz-Mu{\~{n}}oz, Dario and Garcia-Carballeira, Felix and Camarmas-Alonso, Diego and Calderon-Mateos, Alejandro and Carretero, Jesus",
   title="Fault Tolerant in the Expand Ad-Hoc Parallel File System",
   booktitle="Euro-Par 2024: Parallel Processing",
   year="2024",
   publisher="Springer Nature Switzerland",
   address="Cham",
   pages="62--76",
   isbn="978-3-031-69766-1"
}
  ```
</details>

<details>
<summary>:newspaper: Malleability in the Expand Ad-Hoc parallel file system</summary>
 
  * Authors: Dario Muñoz-Muñoz, Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos, Jesus Carretero
  * Conference paper: 3rd EuroHPC Workshop on Dynamic Resources in HPC. Euro-Par 2024
  * Link: [:link: Open publication](https://doi.org/10.1007/978-3-031-90200-0_26)
  * Cite:
  ```bash
@InProceedings{10.1007/978-3-031-90200-0_26,
   author="Mu{\~{n}}oz-Mu{\~{n}}oz, Dario and Garcia-Carballeira, Felix and Camarmas-Alonso, Diego and Calderon-Mateos, Alejandro and Carretero, Jesus",
   title="Malleability in the Expand Ad-Hoc Parallel File System",
   booktitle="Euro-Par 2024: Parallel Processing Workshops",
   year="2025",
   publisher="Springer Nature Switzerland",
   address="Cham",
   pages="322--333",
   isbn="978-3-031-90200-0"
}
  ```
</details>

<details>
<summary>:newspaper: Tolerancia a fallos en el sistema de ficheros paralelo Expand Ad-Hoc</summary>
 
  * Authors: Dario Muñoz-Muñoz, Diego Camarmas-Alonso, Felix Garcia-Carballeira, Alejandro Calderon-Mateos, Jesus Carretero
  * Conference paper: XXXIV Jornadas de Paralelismo (JP24)
  * Link: [:link: Open publication](http://dx.doi.org/10.5281/zenodo.12743582)
  * Cite:
  ```bash
@inproceedings{munoz_munoz_2024_12743583,
  author       = {Muñoz-Muñoz, Dario and  Camarmas Alonso, Diego and  Garcia-Carballeira, Felix and Calderon-Mateos, Alejandro and Carretero, Jesus},
  title        = {Tolerancia a fallos en el sistema de ficheros paralelo Expand Ad-Hoc},
  booktitle    = {Avances en Arquitectura y Tecnología de Computadores. Actas de las Jornadas SARTECO},
  year         = 2024,
  pages        = {271-279},
  publisher    = {Zenodo},
  month        = jul,
  venue        = {A Coruña, España},
  doi          = {10.5281/zenodo.12743583},
  url          = {https://doi.org/10.5281/zenodo.12743583},
}
  ```
</details>

<details>
<summary>:newspaper: Evaluación del rendimiento de un sistema de ficheros para sistemas IoT</summary>
 
  * Authors: Elias Del-Pozo-Puñal, Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos
  * Conference paper: XXXIV Jornadas de Paralelismo (JP24)
  * Link: [:link: Open publication](http://dx.doi.org/10.5281/zenodo.12094741)
  * Cite:
  ```bash
@inproceedings{del_pozo_punal_2024_12094742,
  author       = {Del-Pozo-Puñal, Elias and  Garcia-Carballeira, Felix and Camarmas-Alonso, Diego and Calderon-Mateos, Alejandro},
  title        = {Evaluación del rendimiento de un sistema de ficheros para sistemas IoT},
  booktitle    = {Avances en Arquitectura y Tecnología de Computadores. Actas de las Jornadas SARTECO},
  year         = 2024,
  pages        = {289-298},
  publisher    = {Zenodo},
  month        = jun,
  venue        = {A Coruña, Galicia, España},
  doi          = {10.5281/zenodo.12094742},
  url          = {https://doi.org/10.5281/zenodo.12094742},
}
  ```
</details>


### 2023

<details>
<summary>:newspaper: A new Ad-Hoc parallel file system for HPC environments based on the Expand parallel file system</summary>
 
  * Authors: Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos, Jesus Carretero
  * Conference paper: 22nd International Symposium on Parallel and Distributed Computing (ISPDC)
  * Link: [:link: Open publication](http://dx.doi.org/10.1109/ISPDC59212.2023.00015)
  * Cite:
  ```bash
@INPROCEEDINGS{10272428,
  author={Garcia-Carballeira, Felix and Camarmas-Alonso, Diego and Caderon-Mateos, Alejandro and Carretero, Jesus},
  booktitle={2023 22nd International Symposium on Parallel and Distributed Computing (ISPDC)}, 
  title={A new Ad-Hoc parallel file system for HPC environments based on the Expand parallel file system}, 
  year={2023},
  volume={},
  number={},
  pages={69-76},
  keywords={File systems;Distributed computing;Ad-Hoc File System;HPC;Parallel I/O},
  doi={10.1109/ISPDC59212.2023.00015}}
  ```
</details>

<details>
<summary>:newspaper: Evaluación de rendimiento del sistema de ficheros paralelo Expand Ad-Hoc en MareNostrum 4</summary>
 
  * Authors: Diego Camarmas-Alonso, Felix Garcia-Carballeira, Alejandro Calderon-Mateos, Jesus Carretero
  * Conference paper: XXXIII Jornadas de Paralelismo (JP23)
  * Link: [:link: Open publication](http://dx.doi.org/10.5281/zenodo.8378956)
  * Cite:
  ```bash
@misc{diego_camarmas_alonso_2023_8378956,
  author       = {Diego Camarmas-Alonso and Felix Garcia-Carballeira and Alejandro Calderon-Mateos and  Jesus Carretero},
  title        = {Evaluación de rendimiento del sistema de ficheros paralelo Expand Ad-Hoc en MareNostrum 4},
  month        = sep,
  year         = 2023,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.8378956},
  url          = {https://doi.org/10.5281/zenodo.8378956},
}
  ```
</details>

<details>
<summary>:newspaper: Sistema de Ficheros Distribuido para IoT basado en Expand</summary>
 
  * Authors:  Elias Del-Pozo-Puñal, Felix Garcia-Carballeira, Diego Camarmas-Alonso, Alejandro Calderon-Mateos
  * Conference paper: XXXIII Jornadas de Paralelismo (JP23)
  * Link: [:link: Open publication](http://dx.doi.org/10.5281/zenodo.10706248)
  * Cite:
  ```bash
@misc{del_pozo_punal_2023_10706248,
  author       = {Del-Pozo-Puñal, Elías and Garcia-Carballeira, Felix andCamarmas-Alonso, Diego andCalderon-Mateos, Alejandro},
  title        = {Sistema de Ficheros Distribuido para IoT basado en Expand},
  month        = sep,
  year         = 2023,
  publisher    = {Zenodo},
  version      = {Version v1},
  doi          = {10.5281/zenodo.10706248},
  url          = {https://doi.org/10.5281/zenodo.10706248},
}
  ```
</details>


### 2022

<details>
<summary>:newspaper: Sistema de almacenamiento Ad-Hoc para entornos HPC basado en el sistema de ficheros paralelo Expand</summary>
 
  * Authors:  Diego Camarmas-Alonso, Felix Garcia-Carballeira, Alejandro Calderon-Mateos, Jesus Carretero
  * Conference paper: XXXII Jornadas de Paralelismo (JP22)
  * Link: [:link: Open publication](http://dx.doi.org/10.5281/zenodo.6862882)
  * Cite:
  ```bash
@misc{diego_camarmas_alonso_2021_14258796,
  author       = {Diego Camarmas-Alonso and Felix Garcia-Carballeira and Alejandro Calderon Mateos and Jesus Carretero Perez},
  title        = {Sistema de almacenamiento Ad-Hoc para entornos HPC basado en el sistema de ficheros paralelo Expand},
  month        = sep,
  year         = 2021,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.14258796},
  url          = {https://doi.org/10.5281/zenodo.14258796},
}
  ```
</details>

</details>

