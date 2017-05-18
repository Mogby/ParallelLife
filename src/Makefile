.PHONY: default
default: all

.PHONY: all
all: main libmylib.so

main: main.o libmylib.so
	gcc -ldl main.o -o main

main.o:
	gcc -c main.c

libmylib.so: mylib.o
	gcc -shared -o libmylib.so mylib.o

mylib.o:
	gcc -c -fPIC mylib.c

.PHONY: clean
clean:
	rm *.o main libmylib.so
