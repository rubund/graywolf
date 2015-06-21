


add_executable(TimberWolfSC acceptt.c     coarseglb.c   debug2.c    findcostf.c  globroute.c  newtemp.c   overlap.c    readcell.c   rowevener.c  sortpin.c  ucxxo1.c  upair.c buildimp.c    configpads.c  debug.c     findrcost.c  graphics.c   outcm.c     parser.c     readnets.c   savewolf.c   steiner.c  ucxxo2.c  urcost.c cell_width.c  configure.c   dimbox.c    findunlap.c  main.c       outpins1.c  paths.c      readpar.c    seagate.c    uc0.c      ucxxp.c   utemp.c cglbroute.c   countf.c      feedest.c   gateswap.c   mergeseg.c   outpins.c   placepads.c  reconfig.c   sort.c       ucxx1.c    uloop.c   xpickint.c changrid.c    crossbus.c    findcost.c  globe.c      netgraph.c   output.c    readblck.c   rmoverlap.c  sortpad.c    ucxx2.c    unlap.c ${CMAKE_SOURCE_DIR}/src/date/date.c)

target_link_libraries(TimberWolfSC ${CMAKE_BINARY_DIR}/src/Ylib/libycadgraywolf.so)
target_link_libraries(TimberWolfSC X11)
target_link_libraries(TimberWolfSC m)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include .)

install(TARGETS TimberWolfSC DESTINATION lib/graywolf/bin)
