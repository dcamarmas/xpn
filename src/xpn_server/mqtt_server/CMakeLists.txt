if(NOT ENABLE_MQTT_SERVER)  
    return()
endif()

file(GLOB MQTT_SERVER_HEADERS 
    "*.hpp"
)

file(GLOB MQTT_SERVER_SOURCE 
    "*.cpp"
)

target_sources(xpn_server PRIVATE ${MQTT_SERVER_HEADERS} ${MQTT_SERVER_SOURCE})
target_link_libraries(xpn_server PRIVATE "${MOSQUITTO_LIBRARY}")
target_include_directories(xpn_server PRIVATE "${MOSQUITTO_INCLUDE_DIR}")