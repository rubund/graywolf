
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(GrayWolfSyntax SHARED
	main.c
	output.c
	readcells.c
	${CMAKE_SOURCE_DIR}/src/date/date.c
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(
	TARGETS GrayWolfSyntax
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

