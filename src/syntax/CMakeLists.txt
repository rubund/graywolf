

add_executable(syntax main.c output.c readcells.c ${CMAKE_SOURCE_DIR}/src/date/date.c)
set_target_properties(syntax PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

target_link_libraries(syntax ycadgraywolf)
target_link_libraries(syntax X11)
target_link_libraries(syntax m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(TARGETS syntax DESTINATION ${CMAKE_INSTALL_LIBDIR}/graywolf/bin)
