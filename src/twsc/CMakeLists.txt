
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

bison_target(readcell readcell.y ${CMAKE_BINARY_DIR}/src/twsc/readcell.y.c)
flex_target(readcell readcell.lex ${CMAKE_BINARY_DIR}/src/twsc/readcell.lex.c)

bison_target(readnets readnets.y ${CMAKE_BINARY_DIR}/src/twsc/readnets.y.c)
flex_target(readnets readnets.lex ${CMAKE_BINARY_DIR}/src/twsc/readnets.lex.c)

add_library(TimberWolfSC SHARED
	${BISON_readcell_OUTPUTS}
	${FLEX_readcell_OUTPUTS}
	${BISON_readnets_OUTPUTS}
	${FLEX_readnets_OUTPUTS}
	readnets_functions.c
	acceptt.c
	coarseglb.c
	debug2.c
	findcostf.c
	globroute.c
	newtemp.c
	overlap.c
	rowevener.c
	sortpin.c
	ucxxo1.c
	upair.c
	buildimp.c
	configpads.c
	debug.c
	findrcost.c
	graphics.c
	outcm.c
	parser.c
	savewolf.c
	steiner.c
	ucxxo2.c
	urcost.c
	cell_width.c
	configure.c
	dimbox.c
	findunlap.c
	main.c
	outpins1.c
	paths.c
	readpar.c
	seagate.c
	uc0.c
	ucxxp.c
	utemp.c
	cglbroute.c
	countf.c
	feedest.c
	gateswap.c
	mergeseg.c
	outpins.c
	placepads.c
	reconfig.c
	sort.c
	ucxx1.c
	uloop.c
	xpickint.c
	changrid.c
	crossbus.c
	findcost.c
	globe.c
	netgraph.c
	output.c
	readblck.c
	rmoverlap.c
	sortpad.c
	ucxx2.c
	unlap.c
	)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/src/twsc .)

install(
	TARGETS TimberWolfSC
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

