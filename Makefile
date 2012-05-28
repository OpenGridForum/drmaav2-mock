DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o
TESTOBJS = test.o tests/test_app.o tests/test_dict.o tests/test_list.o tests/test_sessions.o tests/test_jsession.o tests/test_msession.o

CC = gcc
CFLAGS =

all: test


tests/%.o: tests/%.c tests/%.h
	$(CC) -c $< -o $@

test: $(DRMAAOBJS) $(TESTOBJS)
	$(CC) -lcunit -o $@ $(DRMAAOBJS) $(TESTOBJS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f tests/*.o test
