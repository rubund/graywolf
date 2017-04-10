
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

bison_target(readcgraph readcgraph.y ${CMAKE_BINARY_DIR}/src/mc_compact/readcgraph.c)
#flex_target(readcgraph .lex ${CMAKE_BINARY_DIR}/src/mc_compact/readcgraph.lex.c COMPILE_FLAGS "-d") #debug
flex_target(readcgraph readcgraph.lex ${CMAKE_BINARY_DIR}/src/mc_compact/readcgraph.lex.c)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(GrayWolfMCCompact SHARED
	${BISON_readcgraph_OUTPUTS}
	${FLEX_readcgraph_OUTPUTS}
	cdraw.c
	compactor.c
	io.c
	multi.c
	stdmacro.c
	changraph.c
	debug2.c
	movestrat2.c
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

