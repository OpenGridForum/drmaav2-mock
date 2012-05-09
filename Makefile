DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o
CC = gcc
CFLAGS =

all: test large test_dict test_list test_sessions test_jsession

test: $(DRMAAOBJS) test.o
	$(CC) -o $@ $(DRMAAOBJS) test.o

test_dict: $(DRMAAOBJS) test_dict.o
	$(CC) -o $@ $(DRMAAOBJS) test_dict.o

test_list: $(DRMAAOBJS) test_list.o
	$(CC) -o $@ $(DRMAAOBJS) test_list.o

large: $(DRMAAOBJS) large.o
	$(CC) -o $@ $(DRMAAOBJS) large.o

test_sessions: $(DRMAAOBJS) test_sessions.o
	$(CC) -o $@ $(DRMAAOBJS) test_sessions.o

test_jsession: $(DRMAAOBJS) test_jsession.o
	$(CC) -o $@ $(DRMAAOBJS) test_jsession.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(DRMAAOBJS) *.o test large test_list test_dict test_sessions test_jsession
