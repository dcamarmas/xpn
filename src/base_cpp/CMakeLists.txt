
file(GLOB XPN_BASE_CPP_HEADERS
    "*.hpp"
)

file(GLOB XPN_BASE_CPP_SOURCE
    "*.cpp"
)

if("${FABRIC_PATH}" STREQUAL "")
    list(FILTER XPN_BASE_CPP_HEADERS EXCLUDE REGEX "fabric")
    list(FILTER XPN_BASE_CPP_SOURCE EXCLUDE REGEX "fabric")
endif()

add_library(xpn_base_cpp OBJECT ${XPN_BASE_CPP_SOURCE} ${XPN_BASE_CPP_HEADERS})
target_include_directories(xpn_base_cpp PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)   

if(NOT "${FABRIC_PATH}" STREQUAL "")
    target_link_libraries(xpn_base_cpp PUBLIC fabric)
    target_link_directories(xpn_base_cpp PUBLIC ${FABRIC_PATH}/lib)
    target_include_directories(xpn_base_cpp PUBLIC 
        ${FABRIC_PATH}/include
    )
endif()