

add_executable(Mincut main.c output.c readcells.c ${CMAKE_SOURCE_DIR}/src/date/date.c)
set_target_properties(Mincut PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

target_link_libraries(Mincut ycadgraywolf)
target_link_libraries(Mincut X11)
target_link_libraries(Mincut m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(TARGETS Mincut DESTINATION ${CMAKE_INSTALL_LIBDIR}/graywolf/bin)
