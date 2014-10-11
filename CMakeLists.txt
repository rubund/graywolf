cmake_minimum_required (VERSION 2.6)
project (graywolf)

INCLUDE(CheckIncludeFiles)


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config-build.h)


install(DIRECTORY flow DESTINATION lib/graywolf/bin)
install(CODE "EXECUTE_PROCESS(COMMAND ln -sf flow.noroute flow WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/graywolf/bin/flow)")

add_subdirectory(src)
add_subdirectory(script)
