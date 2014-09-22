
CFLAGS := -I../../include -I. -g

OBJ := main.o  output.o  readcells.o


default: $(OBJ) date.o
	gcc -o syntax $(OBJ) date.o ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o syntax

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean

