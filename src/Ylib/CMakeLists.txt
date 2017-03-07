
add_library(ycadgraywolf SHARED
	assign.c
	buster.c
	cleanup.c
	colors.c
	deck.c
	dialog.c
	draw.c
	dset.c
	edcolors.c
	file.c
	getftime.c
	graph.c
	grid.c
	hash.c
	heap.c
	list.c
	log.c
	matrix.c
	menus.c
	message.c
	mst.c
	mytime.c
	okmalloc.c
	path.c
	plot.c
	program.c
	project.c
	queue.c
	quicksort.c
	radixsort.c
	rand.c
	rbtree.c
	relpath.c
	set.c
	stat.c
	stats.c
	string.c
	svd.c
	system.c
	time.c
	timer.c
	trans.c
	wgraphics.c
	ydebug.c
	yreadpar.c
	)

set_target_properties(ycadgraywolf PROPERTIES VERSION 0.0.0 SOVERSION 0)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)

install(
	TARGETS ycadgraywolf
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

