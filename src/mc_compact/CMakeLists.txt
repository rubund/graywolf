
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

bison_target(readcgraph readcgraph.y ${CMAKE_BINARY_DIR}/src/mc_compact/readcgraph.y.c)
flex_target(readcgraph readcgraph.lex ${CMAKE_BINARY_DIR}/src/mc_compact/readcgraph.lex.c)

bison_target(readtiles readtiles.y ${CMAKE_BINARY_DIR}/src/mc_compact/readtiles.y.c)
flex_target(readtiles readtiles.lex ${CMAKE_BINARY_DIR}/src/mc_compact/readtiles.lex.c)

add_library(GrayWolfMCCompact SHARED
	${BISON_readcgraph_OUTPUTS}
	${FLEX_readcgraph_OUTPUTS}
	${BISON_readtiles_OUTPUTS}
	${FLEX_readtiles_OUTPUTS}
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
	ycompact.c
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

install(
	TARGETS GrayWolfMCCompact
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

