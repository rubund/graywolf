cmake_minimum_required (VERSION 2.6)
project (mountainwolf)

INCLUDE(CheckIncludeFiles)


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)
