if(NOT ENABLE_FABRIC_SERVER)  
    return()
endif()

file(GLOB FABRIC_SERVER_HEADERS 
    "*.hpp"
)

file(GLOB FABRIC_SERVER_SOURCE 
    "*.cpp"
)

target_sources(xpn_server PRIVATE ${FABRIC_SERVER_HEADERS} ${FABRIC_SERVER_SOURCE})
target_link_libraries(xpn_server PRIVATE lfi)