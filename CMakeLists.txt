cmake_minimum_required (VERSION 2.6)
project (graywolf)

find_package(PkgConfig)
find_package(X11)
INCLUDE(CheckIncludeFiles)


pkg_check_modules(GSL gsl)

if (NOT GSL_FOUND)
  MESSAGE(FATAL_ERROR "The development files for the GNU Scientific Library (libgsl) are required to build graywolf.")
endif()

# Include RPATH in build so that ldconfig is not necessary after install
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config-build.h)


install(DIRECTORY flow DESTINATION lib/graywolf/bin)
install(CODE "EXECUTE_PROCESS(COMMAND ln -sf flow.noroute flow WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/graywolf/bin/flow)")

add_subdirectory(src)
add_subdirectory(script)
