

add_executable(Mincut main.c output.c readcells.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(Mincut ${CMAKE_BINARY_DIR}/src/Ylib/libycadgraywolf.so)
target_link_libraries(Mincut X11)
target_link_libraries(Mincut m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(TARGETS Mincut DESTINATION lib/graywolf/bin)
