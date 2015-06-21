

add_executable(TimberWolfMC acceptt.c analyze.c compact.c config1.c config2.c configpads.c debug.c finalout.c finalpin.c gmain.c findcheck.c findcost.c findloc.c findside.c fixcell.c genorient.c graphics.c initialize.c initnets.c loadbins.c main.c makebins.c makesite.c mergecell.c neworient.c newtemp.c outgeo.c outpin.c output.c overlap.c paths.c partition.c penalties.c perimeter.c placepads.c placepin.c prboard.c readcells.c readnets.c rmain.c readpar.c reconfig.c savewolf.c scrapnet.c sortpin.c selectpin.c setpwates.c sortpad.c twstats.c uaspect.c uloop.c unbust.c uinst.c unet.c upin.c upinswap.c usite1.c usite2.c usoftmove.c utemp.c watesides.c window.c wirecosts.c wireest.c wireratio.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(TimberWolfMC ${CMAKE_BINARY_DIR}/src/Ylib/libycadgraywolf.so)
target_link_libraries(TimberWolfMC X11)
target_link_libraries(TimberWolfMC m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(TARGETS TimberWolfMC DESTINATION lib/graywolf/bin)
