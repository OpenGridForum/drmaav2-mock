DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o
CC = gcc
CFLAGS =

all: test test_sessions test_jsession test_msession


test_sessions: $(DRMAAOBJS) test_sessions.o
	$(CC) -o $@ $(DRMAAOBJS) test_sessions.o

test_jsession: $(DRMAAOBJS) test_jsession.o
	$(CC) -o $@ $(DRMAAOBJS) test_jsession.o

test_msession: $(DRMAAOBJS) test_msession.o
	$(CC) -o $@ $(DRMAAOBJS) test_msession.o


tests/test_app.o: tests/test_app.c
	$(CC) -c tests/test_app.c -o tests/test_app.o	

tests/test_dict.o: tests/test_dict.c
	$(CC) -c tests/test_dict.c -o tests/test_dict.o

tests/test_list.o: tests/test_list.c
	$(CC) -c tests/test_list.c -o tests/test_list.o

test: $(DRMAAOBJS) test.o tests/test_app.o tests/test_dict.o tests/test_list.o
	$(CC) -lcunit -o $@ $(DRMAAOBJS) test.o tests/test_app.o tests/test_dict.o tests/test_list.o


%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(DRMAAOBJS) *.o test test_sessions test_jsession test_msession
	rm -f tests/*.o tests/teat_app tests/test_dict tests/test_list
