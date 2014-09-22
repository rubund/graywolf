
CFLAGS := -I../../include -I. -g

default: graphics.o readnet.o date.o main.o
	gcc -o analyze_nets graphics.o main.o readnet.o date.o ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o analyze_nets

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean

