cmake_minimum_required (VERSION 2.6)
project (graywolf)

INCLUDE(GNUInstallDirs)
INCLUDE(CheckIncludeFiles)

# Include RPATH in build so that ldconfig is not necessary after install

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config-build.h)

install(
	DIRECTORY flow
	DESTINATION ${CMAKE_INSTALL_DATADIR}/graywolf
	)

#install(CODE "EXECUTE_PROCESS(COMMAND ln -sf flow.noroute flow WORKING_DIRECTORY ${SHARE_INSTALL_PREFIX}/graywolf/flow)")

add_subdirectory(src)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

