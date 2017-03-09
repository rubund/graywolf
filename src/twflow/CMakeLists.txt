
add_executable(graywolf
	autoflow.c
	findflow.c
	graphics.c
	io.c
	main.c
	program.c
	readobjects.c
	${CMAKE_SOURCE_DIR}/src/date/date.c
	)

target_link_libraries(graywolf ycadgraywolf)
target_link_libraries(graywolf GraywolfMincut)
target_link_libraries(graywolf TimberWolfMC)
target_link_libraries(graywolf GrayWolfSyntax)
target_link_libraries(graywolf X11)
target_link_libraries(graywolf m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(
	TARGETS graywolf
	DESTINATION ${CMAKE_INSTALL_BINDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)

