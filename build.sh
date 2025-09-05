
set -x
set -e

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <build_directory> <install_directory>"
    exit 1
fi

cd $1

cmake -S .. -B . \
    -D BUILD_TESTS=true \
    -D CMAKE_INSTALL_PREFIX=$2 \
    -D CMAKE_C_COMPILER=gcc \
    -D CMAKE_CXX_COMPILER=g++ \
    -D ENABLE_MPI_SERVER=off \
    -D ENABLE_FABRIC_SERVER=off \
    -D ENABLE_MQ_SERVER=off

# cmake -S .. -B . \
#     -D BUILD_TESTS=true \
#     -D CMAKE_INSTALL_PREFIX=$2 \
#     -D CMAKE_C_COMPILER=gcc \
#     -D CMAKE_CXX_COMPILER=g++ \
#     -D ENABLE_SCK_SERVER=on \
#     -D ENABLE_MPI_SERVER=off \
#     -D ENABLE_FABRIC_SERVER=off \
#     -D ENABLE_MQ_SERVER=/usr/lib/x86_64-linux-gnu

# cmake -S .. -B . \
#     -D BUILD_TESTS=true \
#     -D CMAKE_INSTALL_PREFIX=$2 \
#     -D CMAKE_C_COMPILER=gcc \
#     -D CMAKE_CXX_COMPILER=g++ \
#     -D ENABLE_MPI_SERVER=off \
#     -D ENABLE_FABRIC_SERVER=/home/lab/bin/libfabric \
#     -D ENABLE_MQ_SERVER=/usr/lib/x86_64-linux-gnu

# cmake -S .. -B . \
#     -D BUILD_TESTS=true \
#     -D CMAKE_INSTALL_PREFIX=$2 \
#     -D CMAKE_C_COMPILER=mpicc \
#     -D CMAKE_CXX_COMPILER=mpic++ \
#     -D ENABLE_MPI_SERVER=/usr/lib/x86_64-linux-gnu/mpich \
#     -D ENABLE_FABRIC_SERVER=off \
#     -D ENABLE_MQ_SERVER=/usr/lib/x86_64-linux-gnu
cmake --build . -j 8 

cmake --install .