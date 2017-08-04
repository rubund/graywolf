
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

