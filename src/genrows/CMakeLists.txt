
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fvisibility=hidden")

add_library(GrayWolfGenRows SHARED
	draw.c
	genrows.c
	main.c
	merge.c
	readpar.c
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(
	TARGETS GrayWolfGenRows
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

