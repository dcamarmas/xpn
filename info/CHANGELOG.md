
# Changelog

## 3.3.1 - 2025-09-10

  * Expand proxy added
  * C3 Platform added
  * Minor updates

## 3.3.0 - 2025-06-05

  * Spark connector added
  * IPv6 support added
  * Added support for 32-bits version of Expand

## 3.2.0 - 2025-03-12

  * MQTT support added: mpi_server, sck_server and now mq_server available
  * Cleanup and minor improvements on socket support
  * Improvements on the configuration file reader
  * Minor updates on examples

## 3.0.0 - 2024-06-05

  * Fault tolerance support based on replication added
  * Maleability support improved
  * Expand deployment based on docker added
  * MXML dependency removed
  * Simplified user experience: more user-friendly way to start and stop ad hoc server
  * Minor bug fixes and improvements

## 2.2.2 - 2023-06-12

  * TCP server rebuilt from scratch
  * New system calls intercepted
  * Preload and flush data operations added
  * Minor bug fixes and improvements

## 2.2.1 - 2023-03-31

  * Minor bug fixes and improvements

## 2.2.0 - 2023-03-24

  * Maleability support added

## 2.1.0 - 2023-03-03

  * Spack support added
  * Simplified user experience: now it is easier to start and stop expand ad-hoc servers
  * Code refactoring

## 2.0.0 - 2022-12-12

  * First XPN Ad-Hoc release
  * This version provides:
    * Simplifiyed build system based on build-me script.  
    * Platforms: MPICH and Intel MPI.
    * Benchmarks tested: IOR, MdTest and IO500.
    * API: POSIX (through syscall interception library) and native XPN (similar to POSIX).
    * Main features: data locality, MPI Ad-Hoc servers, thread on-demand or thread pool for MPI servers
