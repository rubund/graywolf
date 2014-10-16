

#add_executable(mincut main.c output.c readcells.c ${CMAKE_SOURCE_DIR}/src/date/date.c)


add_library(ycad.sys SHARED assign.c buster.c cleanup.c colors.c deck.c dialog.c draw.c dset.c edcolors.c file.c getftime.c graph.c grid.c hash.c heap.c list.c log.c matrix.c menus.c message.c mst.c mytime.c okmalloc.c path.c plot.c program.c project.c queue.c quicksort.c radixsort.c rand.c rbtree.c relpath.c set.c stat.c stats.c string.c svd.c system.c time.c timer.c trans.c wgraphics.c ydebug.c yreadpar.c )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)

install(TARGETS ycad.sys DESTINATION lib)
