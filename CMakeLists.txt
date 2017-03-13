cmake_minimum_required (VERSION 2.6)
project (graywolf)

INCLUDE(GNUInstallDirs)
INCLUDE(CheckIncludeFiles)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config-build.h)

install(
	DIRECTORY flow
	DESTINATION ${CMAKE_INSTALL_DATADIR}/graywolf
	)

add_subdirectory(src)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

