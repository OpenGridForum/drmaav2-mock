DRMAAOBJS = drmaa2.o drmaa2-list.o 
CC = gcc
CFLAGS =

all: test large

test: $(DRMAAOBJS) test.o
	$(CC) -o $@ $(DRMAAOBJS) test.o

large: $(DRMAAOBJS) large.o
	$(CC) -o $@ $(DRMAAOBJS) large.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(DRMAAOBJS) test.o test large.o large
