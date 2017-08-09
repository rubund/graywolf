
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

bison_target(mincutparser readcells.y ${CMAKE_BINARY_DIR}/src/mincut/readcells.y.c)
flex_target(mincutparser readcells.lex ${CMAKE_BINARY_DIR}/src/mincut/readcells.lex.c)

add_library(GraywolfMincut SHARED
	${BISON_mincutparser_OUTPUTS}
	${FLEX_mincutparser_OUTPUTS}
	main.c
	output.c
	separate.c
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/src/mincut)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

install(
	TARGETS GraywolfMincut
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

