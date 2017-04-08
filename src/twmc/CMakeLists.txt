
INCLUDE(FindBISON)
INCLUDE(FindFLEX)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

bison_target(twmcreadcells readcells.y ${CMAKE_BINARY_DIR}/src/twmc/readcells.c)
# flex_target(twmcreadcells readcells.lex ${CMAKE_BINARY_DIR}/src/twmc/readcells.lex.c)
flex_target(twmcreadcells readcells.lex ${CMAKE_BINARY_DIR}/src/twmc/readcells.lex.c COMPILE_FLAGS "-d") #debug

bison_target(twmcreadnets readnets.y ${CMAKE_BINARY_DIR}/src/twmc/readnets.c)
flex_target(twmcreadnets readnets.lex ${CMAKE_BINARY_DIR}/src/twmc/readnets.lex.c)
# flex_target(twmcreadnets readnets.lex ${CMAKE_BINARY_DIR}/src/twmc/readnets.lex.c COMPILE_FLAGS "-d") #debug

add_library(TimberWolfMC SHARED
	${BISON_twmcreadcells_OUTPUTS}
	${FLEX_twmcreadcells_OUTPUTS}
	${BISON_twmcreadnets_OUTPUTS}
	${FLEX_twmcreadnets_OUTPUTS}
	acceptt.c
	analyze.c
	compact.c
	config1.c
	config2.c
	configpads.c
	debug.c
	finalout.c
	finalpin.c
	gmain.c
	findcheck.c
	findcost.c
	findloc.c
	findside.c
	fixcell.c
	genorient.c
	graphics.c
	initialize.c
	initnets.c
	loadbins.c
	main.c
	makebins.c
	makesite.c
	mergecell.c
	neworient.c
	newtemp.c
	outgeo.c
	outpin.c
	output.c
	overlap.c
	paths.c
	partition.c
	penalties.c
	perimeter.c
	placepads.c
	placepin.c
	prboard.c
	rmain.c
	readpar.c
	reconfig.c
	savewolf.c
	scrapnet.c
	sortpin.c
	selectpin.c
	setpwates.c
	sortpad.c
	twstats.c
	uaspect.c
	uloop.c
	unbust.c
	uinst.c
	unet.c
	upin.c
	upinswap.c
	usite1.c
	usite2.c
	usoftmove.c
	utemp.c
	watesides.c
	window.c
	wirecosts.c
	wireest.c
	wireratio.c
	${CMAKE_SOURCE_DIR}/src/date/date.c
	)

target_link_libraries(TimberWolfMC GrayWolfMCCompact)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/src/twmc .)

install(
	TARGETS TimberWolfMC
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

