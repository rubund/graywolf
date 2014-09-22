
CFLAGS := -I../../include -I. -g

OBJ := draw.o  genrows.o  main.o  merge.o  readpar.o


default: $(OBJ) date.o
	gcc -o genrows $(OBJ) date.o ../Ylib/libycad.mem.a ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o genrows

binary.o: binary.c
	gcc  -Wno-traditional $(CFLAGS) -g -c -o binary.o binary.c

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean

