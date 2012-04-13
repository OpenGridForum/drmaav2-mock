DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o
CC = gcc
CFLAGS =

all: test large test_dict test_list

test: $(DRMAAOBJS) test.o
	$(CC) -o $@ $(DRMAAOBJS) test.o

test_dict: $(DRMAAOBJS) test_dict.o
	$(CC) -o $@ $(DRMAAOBJS) test_dict.o

test_list: $(DRMAAOBJS) test_list.o
	$(CC) -o $@ $(DRMAAOBJS) test_list.o

large: $(DRMAAOBJS) large.o
	$(CC) -o $@ $(DRMAAOBJS) large.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(DRMAAOBJS) *.o test large test_list test_dict
