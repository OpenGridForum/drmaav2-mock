all: test

test: test.o drmaa2.o
	gcc -o test test.o drmaa2.o

drmaa2.o: drmaa2.c drmaa2.h
	gcc -c drmaa2.c

test.o: test.c
	gcc -c test.c

clean:
	rm -f drmaa2.o test.o test