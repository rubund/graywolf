
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fvisibility=hidden")

bison_target(syntaxparser readcells.y ${CMAKE_BINARY_DIR}/src/syntax/readcells.c COMPILE_FLAGS "--token-table")
flex_target(syntaxparser readcells.lex ${CMAKE_BINARY_DIR}/src/syntax/readcells.lex.c)

add_library(GrayWolfSyntax SHARED
	main.c
	output.c
	${BISON_syntaxparser_OUTPUTS}
	${FLEX_syntaxparser_OUTPUTS}
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/src/syntax .)

install(
	TARGETS GrayWolfSyntax
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

