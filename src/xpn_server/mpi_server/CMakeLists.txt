if(NOT ENABLE_MPI_SERVER)  
    return()
endif()

file(GLOB MPI_SERVER_HEADERS 
    "*.hpp"
)

file(GLOB MPI_SERVER_SOURCE 
    "*.cpp"
)

target_sources(xpn_server PRIVATE ${MPI_SERVER_HEADERS} ${MPI_SERVER_SOURCE})
target_link_libraries(xpn_server PRIVATE "${MPI_LIBRARY}")
target_include_directories(xpn_server PRIVATE "${MPI_INCLUDE_DIR}")