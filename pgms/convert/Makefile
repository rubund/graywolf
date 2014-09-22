
CFLAGS := -I../../include -g

default: convert.o main.o pads.o
	gcc -o convert convert.o main.o pads.o

clean:
	rm -rf *.o convert

.PHONY: default clean
