
if(ENABLE_MPI_SERVER)
	link_libraries("${MPI_LIBRARY}")
	include_directories("${MPI_INCLUDE_DIR}")
endif()
if(ENABLE_FABRIC_SERVER)
	link_libraries(lfi)
endif()

add_subdirectory(nfi)
add_subdirectory(xpn)

file(GLOB XPN_HEADERS
	"*.h"
)

file(GLOB XPN_SOURCE
	"*.cpp"
)

add_library(xpn STATIC ${XPN_HEADERS} ${XPN_SOURCE})
target_link_libraries(xpn PRIVATE xpn_base_cpp xpn_core xpn_nfi)

add_library(xpn_shared SHARED $<TARGET_OBJECTS:xpn>)
set_target_properties(xpn_shared PROPERTIES OUTPUT_NAME xpn)
target_link_libraries(xpn_shared PRIVATE xpn_base_cpp xpn_core xpn_nfi)

target_include_directories(xpn PUBLIC
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/xpn_client"
)

if(ENABLE_MPI_SERVER)
	target_link_libraries(xpn PUBLIC "${MPI_LIBRARY}")
	target_include_directories(xpn PUBLIC "${MPI_INCLUDE_DIR}")
endif()
if(ENABLE_FABRIC_SERVER)
	target_link_libraries(xpn PUBLIC lfi)
endif()

install(TARGETS xpn LIBRARY DESTINATION lib)
install(TARGETS xpn_shared LIBRARY DESTINATION lib)
install(FILES "xpn.h" DESTINATION include)
