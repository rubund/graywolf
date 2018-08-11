cmake_minimum_required (VERSION 2.8.5)
project (graywolf)

enable_testing()

find_package(PkgConfig)
find_package(X11)
INCLUDE(CheckIncludeFiles)
include(GNUInstallDirs)


pkg_check_modules(GSL gsl)

if (NOT GSL_FOUND)
  MESSAGE(FATAL_ERROR "The development files for the GNU Scientific Library (libgsl) are required to build graywolf.")
endif()

if (NOT X11_FOUND)
  MESSAGE(FATAL_ERROR "The development files for X11 (libx11-dev) are required to build graywolf.")
endif()

# Include RPATH in build so that ldconfig is not necessary after install
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

MATH(EXPR SIZEOF_VOID_P_BITS "${CMAKE_SIZEOF_VOID_P}*8")

# Ignore no-implicit-function-declaration warnings for now, since there are so many - and it then becomes
# easier to see other warnings popping up.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSIZEOF_VOID_P=${SIZEOF_VOID_P_BITS} -Wno-implicit-function-declaration")


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config-build.h)


install(DIRECTORY flow DESTINATION lib/graywolf/bin)
install(CODE "EXECUTE_PROCESS(COMMAND ln -sf flow.noroute flow WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/graywolf/bin/flow)")

add_subdirectory(src)
add_subdirectory(script)
add_subdirectory(tests)

