
add_executable(xpn_reconnect xpn_reconnect.cpp)
target_link_libraries(xpn_reconnect xpn)

install(TARGETS xpn_reconnect RUNTIME DESTINATION test/integrity/xpn_reconnect)