
file(GLOB_RECURSE XPN_CONTROLLER_HEADERS 
    "*.hpp"
)

file(GLOB_RECURSE XPN_CONTROLLER_SOURCE 
    "*.cpp"
)

# add_compile_options("-DDEBUG")
add_executable(xpn_controller ${XPN_CONTROLLER_SOURCE})

target_link_libraries(xpn_controller PRIVATE xpn_base_c xpn_base_cpp xpn)

target_include_directories(xpn_controller PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/xpn_client"
)

install(TARGETS xpn_controller RUNTIME DESTINATION bin)