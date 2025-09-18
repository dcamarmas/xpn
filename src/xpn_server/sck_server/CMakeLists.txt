
file(GLOB SCK_SERVER_HEADERS 
    "*.hpp"
)

file(GLOB SCK_SERVER_SOURCE 
    "*.cpp"
)

target_sources(xpn_server PRIVATE ${SCK_SERVER_HEADERS} ${SCK_SERVER_SOURCE})