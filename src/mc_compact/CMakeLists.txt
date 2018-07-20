


add_executable(mc_compact cdraw.c      compactor.c  io.c          multi.c       stdmacro.c changraph.c  debug2.c     movestrat2.c  readcgraph.c  xcompact.c cmain.c      grid.c       movestrat.c   readtiles.c   ycompact.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(mc_compact ycadgraywolf)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(TARGETS mc_compact DESTINATION lib/graywolf/bin)
