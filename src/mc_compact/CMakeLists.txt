
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(GrayWolfMCCompact SHARED
	cdraw.c
	compactor.c
	io.c
	multi.c
	stdmacro.c
	changraph.c
	debug2.c
	movestrat2.c
	readcgraph.c
	xcompact.c
	cmain.c
	grid.c
	movestrat.c
	readtiles.c
	ycompact.c
	${CMAKE_SOURCE_DIR}/src/date/date.c
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(
	TARGETS GrayWolfMCCompact
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

