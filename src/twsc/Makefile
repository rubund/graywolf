
CFLAGS := -I../../include -I. -g

OBJ := acceptt.o     coarseglb.o   debug2.o    findcostf.o  globroute.o  newtemp.o   overlap.o    readcell.o   rowevener.o  sortpin.o  ucxxo1.o  upair.o \
buildimp.o    configpads.o  debug.o     findrcost.o  graphics.o   outcm.o     parser.o     readnets.o   savewolf.o   steiner.o  ucxxo2.o  urcost.o \
cell_width.o  configure.o   dimbox.o    findunlap.o  main.o       outpins1.o  paths.o      readpar.o    seagate.o    uc0.o      ucxxp.o   utemp.o \
cglbroute.o   countf.o      feedest.o   gateswap.o   mergeseg.o   outpins.o   placepads.o  reconfig.o   sort.o       ucxx1.o    uloop.o   xpickint.o \
changrid.o    crossbus.o    findcost.o  globe.o      netgraph.o   output.o    readblck.o   rmoverlap.o  sortpad.o    ucxx2.o    unlap.o 


default: $(OBJ) date.o
	gcc -o TimberWolfSC $(OBJ) date.o ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o TimberWolfSC

install: default
	install TimberWolfSC $(DESTDIR)$(libdir)/timberwolf/bin/

binary.o: binary.c
	gcc  -Wno-traditional $(CFLAGS) -g -c -o binary.o binary.c

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean install

