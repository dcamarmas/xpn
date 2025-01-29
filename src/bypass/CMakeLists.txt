
add_library(xpn_bypass SHARED "xpn_bypass.cpp")

set_target_properties(xpn_bypass PROPERTIES PREFIX "")

target_link_libraries(xpn_bypass PRIVATE xpn pthread dl )

target_include_directories(xpn_bypass PRIVATE 
    "${PROJECT_SOURCE_DIR}/src"
)

install(TARGETS xpn_bypass LIBRARY DESTINATION lib)