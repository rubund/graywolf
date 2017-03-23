
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

bison_target(parser readcells_parser.y ${CMAKE_BINARY_DIR}/src/twmc_readcells/readcells_parser.y.c)
flex_target(parser readcells_parser.lex ${CMAKE_BINARY_DIR}/src/twmc_readcells/lex.yy.c)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(TimberWolfMCReadCells SHARED
	${BISON_parser_OUTPUTS}
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(
	TARGETS TimberWolfMCReadCells
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

