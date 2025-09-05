if(NOT ENABLE_MQ_SERVER)  
    return()
endif()

file(GLOB MQ_SERVER_HEADERS 
    "*.hpp"
)

file(GLOB MQ_SERVER_SOURCE 
    "*.cpp"
)

target_sources(xpn_server PRIVATE ${MQ_SERVER_HEADERS} ${MQ_SERVER_SOURCE})
target_link_libraries(xpn_server PRIVATE "${MOSQUITTO_LIBRARY}")
target_include_directories(xpn_server PRIVATE "${MOSQUITTO_INCLUDE_DIR}")