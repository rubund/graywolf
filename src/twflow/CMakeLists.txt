

add_executable(twflow autoflow.c findflow.c graphics.c io.c main.c program.c readobjects.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(twflow ${CMAKE_BINARY_DIR}/src/Ylib/libycad.sys.a)
target_link_libraries(twflow X11)
target_link_libraries(twflow m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include .)

install(TARGETS twflow DESTINATION lib/graywolf/bin)
