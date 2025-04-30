
add_executable(fuse-xpn "fuse-xpn.cpp")

target_link_libraries(fuse-xpn PRIVATE xpn fuse)

# TODO: change hardcode paths
target_link_directories(fuse-xpn PRIVATE 
    "/beegfs/home/javier.garciablas/dariomnz/bin/libfuse/lib"
)

target_include_directories(fuse-xpn PRIVATE 
    "/beegfs/home/javier.garciablas/dariomnz/bin/libfuse/include"
)

target_compile_options(fuse-xpn PRIVATE "-D_FILE_OFFSET_BITS=64")

install(TARGETS fuse-xpn RUNTIME DESTINATION bin)