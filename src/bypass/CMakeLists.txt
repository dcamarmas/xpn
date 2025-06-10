
add_library(xpn_bypass SHARED "xpn_bypass.cpp")

set_target_properties(xpn_bypass PROPERTIES PREFIX "")

target_link_libraries(xpn_bypass PRIVATE xpn pthread dl )

target_include_directories(xpn_bypass PRIVATE 
    "${PROJECT_SOURCE_DIR}/src"
)

install(TARGETS xpn_bypass LIBRARY DESTINATION lib)

if(DMTCP_PATH)
    message(STATUS "Building DMTCP pluging")
    set(DMTCP_INCLUDE "${DMTCP_PATH}/include")
    set(DMTCP_PLUGIN "xpn_dmtcp")
    set(LIB_SOURCES "xpn_dmtcp.cpp" "xpn_bypass_dmtcp.cpp")
    add_library(${DMTCP_PLUGIN} SHARED ${LIB_SOURCES})
    set_target_properties(${DMTCP_PLUGIN} PROPERTIES PREFIX "")
    target_include_directories(${DMTCP_PLUGIN} PUBLIC 
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/xpn_client"
        ${DMTCP_INCLUDE} 
    )
    target_compile_definitions(${DMTCP_PLUGIN} PUBLIC DMTCP) # -DDMTCP
    target_link_libraries(${DMTCP_PLUGIN} PUBLIC rt xpn pthread dl)

    install(TARGETS ${DMTCP_PLUGIN} LIBRARY DESTINATION lib)

    set(DMTCP_PLUGIN "lfi_dmtcp")
    set(LIB_SOURCES "lfi_bypass.cpp")
    add_library(${DMTCP_PLUGIN} SHARED ${LIB_SOURCES})
    set_target_properties(${DMTCP_PLUGIN} PROPERTIES PREFIX "")
    target_include_directories(${DMTCP_PLUGIN} PUBLIC 
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/xpn_client"
        ${DMTCP_INCLUDE}
        "${DMTCP_PATH}/../mpi-proxy-split/lower-half"
    )
    target_compile_definitions(${DMTCP_PLUGIN} PUBLIC DMTCP) # -DDMTCP
    target_link_libraries(${DMTCP_PLUGIN} PUBLIC rt xpn pthread dl)

    install(TARGETS ${DMTCP_PLUGIN} LIBRARY DESTINATION lib)
    
endif(DMTCP_PATH)