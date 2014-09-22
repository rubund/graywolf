
CFLAGS := -I../../include -I. -g

OBJ := 23tree.o     dens.o       fulllink.o  gmain.o     hprobes.o   polar.o     readgeo.o  reduceg.o  xgraph.o changraph.o  findnodes.o  gentwf.o    graphics.o  makelink.o  printgph.o  rectlin.o  vprobes.o  ygraph.o


default: $(OBJ) date.o
	gcc -o gengraph $(OBJ) date.o ../Ylib/libycad.mem.a ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o gengraph

binary.o: binary.c
	gcc  -Wno-traditional $(CFLAGS) -g -c -o binary.o binary.c

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean

