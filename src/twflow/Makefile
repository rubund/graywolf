
CFLAGS := -I../../include -I. -g

OBJ := autoflow.o  findflow.o  graphics.o  io.o  main.o  program.o  readobjects.o


default: $(OBJ) date.o
	gcc -o twflow $(OBJ) date.o ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o twflow

binary.o: binary.c
	gcc  -Wno-traditional $(CFLAGS) -g -c -o binary.o binary.c

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean

