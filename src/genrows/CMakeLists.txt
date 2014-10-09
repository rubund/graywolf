

add_executable(genrows draw.c genrows.c main.c merge.c readpar.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(genrows ${CMAKE_BINARY_DIR}/src/Ylib/libycad.sys.a)
target_link_libraries(genrows X11)
target_link_libraries(genrows m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include .)
