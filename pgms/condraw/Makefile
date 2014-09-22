
CFLAGS := -I../../include -I. -g

default: ascii.o binary.o main.o mif.o date.o
	gcc -o condraw ascii.o binary.o main.o mif.o date.o ../Ylib/libycad.mem.a ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm


binary.o: binary.c
	gcc  -Wno-traditional $(CFLAGS) -g -c -o binary.o binary.c

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default

