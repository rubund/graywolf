
CFLAGS := -I../../include -I. -g
O=

OBJ= ${O}acceptt.o ${O}analyze.o  \
	${O}compact.o ${O}config1.o ${O}config2.o \
	${O}configpads.o ${O}debug.o \
	${O}finalout.o ${O}finalpin.o ${O}gmain.o \
	${O}findcheck.o ${O}findcost.o ${O}findloc.o \
	${O}findside.o ${O}fixcell.o ${O}genorient.o \
	${O}graphics.o ${O}initialize.o ${O}initnets.o ${O}loadbins.o \
	${O}main.o ${O}makebins.o ${O}makesite.o \
	${O}mergecell.o ${O}neworient.o ${O}newtemp.o \
	${O}outgeo.o ${O}outpin.o ${O}output.o \
	${O}overlap.o ${O}paths.o \
	${O}partition.o ${O}penalties.o ${O}perimeter.o \
	${O}placepads.o ${O}placepin.o ${O}prboard.o \
	${O}readcells.o ${O}readnets.o ${O}rmain.o\
	${O}readpar.o ${O}reconfig.o ${O}savewolf.o \
	${O}scrapnet.o ${O}sortpin.o ${O}selectpin.o \
	${O}setpwates.o ${O}sortpad.o \
	${O}twstats.o ${O}uaspect.o ${O}uloop.o \
	${O}unbust.o ${O}uinst.o ${O}unet.o \
	${O}upin.o ${O}upinswap.o ${O}usite1.o \
	${O}usite2.o ${O}usoftmove.o ${O}utemp.o \
	${O}watesides.o ${O}window.o ${O}wirecosts.o \
	${O}wireest.o ${O}wireratio.o

default: ${OBJ} date.o
	gcc -o TimberWolfMC ${OBJ} date.o $(CFLAGS) ../Ylib/libycad.mem.a ../Ylib/libycad.sys.a -L/usr/X11R6/lib64 -lc -lX11 -lm

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)

clean:
	rm -rf *.o TimberWolfMC

install: default
	install TimberWolfMC $(DESTDIR)$(libdir)/timberwolf/bin/

.PHONY: default clean install
