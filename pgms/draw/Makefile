
CFLAGS := -I../../include -I. -g 


default: cif.o colors.o main.o read.o rtf.o window_manager.o date.o
	gcc -o draw cif.o colors.o main.o read.o rtf.o window_manager.o date.o ../Ylib/libycad.mem.a ../Ylib/libycad.sys.a  -L/usr/X11R6/lib64 -lc -lX11 -lm

clean:
	rm -rf *.o draw

date.o: ../date/date.c
	gcc -c ../date/date.c -o date.o $(CFLAGS)


.PHONY: default clean

