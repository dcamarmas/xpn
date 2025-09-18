if(NOT ENABLE_MPI_SERVER)  
    return()
endif()

file(GLOB MPI_SERVER_HEADERS 
    "*.hpp"
)

file(GLOB MPI_SERVER_SOURCE 
    "*.cpp"
)

target_sources(xpn_nfi PRIVATE ${MPI_SERVER_HEADERS} ${MPI_SERVER_SOURCE})
target_link_libraries(xpn_nfi PUBLIC "${MPI_LIBRARY}")
target_include_directories(xpn_nfi PUBLIC "${MPI_INCLUDE_DIR}")