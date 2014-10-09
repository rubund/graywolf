

add_executable(mincut main.c output.c readcells.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(mincut ${CMAKE_BINARY_DIR}/src/Ylib/libycad.sys.a)
target_link_libraries(mincut X11)
target_link_libraries(mincut m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include .)

install(TARGETS mincut DESTINATION lib/mountainwolf/bin)
