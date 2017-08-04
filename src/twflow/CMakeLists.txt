
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

bison_target(twflowparser readobjects.y ${CMAKE_BINARY_DIR}/src/twflow/readobjects.c)
flex_target(twflowparser readobjects.lex ${CMAKE_BINARY_DIR}/src/twflow/readobjects.lex.c)

add_executable(graywolf
	autoflow.c
	findflow.c
	graphics.c
	io.c
	main.c
	program.c
	${BISON_twflowparser_OUTPUTS}
	${FLEX_twflowparser_OUTPUTS}
	)

target_link_libraries(graywolf ycadgraywolf)
target_link_libraries(graywolf GraywolfMincut)
target_link_libraries(graywolf TimberWolfMC)
target_link_libraries(graywolf TimberWolfSC)
target_link_libraries(graywolf GrayWolfSyntax)
target_link_libraries(graywolf GrayWolfGenRows)
target_link_libraries(graywolf GrayWolfMCCompact)
target_link_libraries(graywolf X11)
target_link_libraries(graywolf m)
target_link_libraries(graywolf gsl)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/src/twflow .)

install(
	TARGETS graywolf
	DESTINATION ${CMAKE_INSTALL_BINDIR}
	)

