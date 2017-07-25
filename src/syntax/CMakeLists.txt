
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -std=c99 -Wdeprecated")

bison_target(syntaxparser readcells.y ${CMAKE_BINARY_DIR}/src/syntax/readcells.c COMPILE_FLAGS "--debug --verbose")
flex_target(syntaxparser readcells.lex ${CMAKE_BINARY_DIR}/src/syntax/readcells.lex.c COMPILE_FLAGS "--debug")

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

