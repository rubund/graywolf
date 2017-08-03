cmake_minimum_required (VERSION 2.6)
project (graywolf)

INCLUDE(GNUInstallDirs)
INCLUDE(CheckIncludeFiles)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
FIND_PACKAGE(GSL REQUIRED)
FIND_PACKAGE(PkgConfig)

pkg_check_modules(GSL gsl)

if (NOT GSL_FOUND)
  MESSAGE(FATAL_ERROR "The development files for the GNU Scientific Library (libgsl) are required to build graywolf.")
endif()

# Include RPATH in build so that ldconfig is not necessary after install
#SET(CMAKE_SKIP_BUILD_RPATH FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Werror-implicit-function-declaration -fvisibility=hidden")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config-build.h)

install(
	DIRECTORY flow
	DESTINATION ${CMAKE_INSTALL_DATADIR}/graywolf
	)

add_subdirectory(src)

