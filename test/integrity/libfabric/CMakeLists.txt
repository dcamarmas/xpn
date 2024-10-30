
add_executable(fabric_test fabric_test.cpp)
target_link_libraries(fabric_test xpn)

install(TARGETS fabric_test RUNTIME DESTINATION test/integrity/libfabric)