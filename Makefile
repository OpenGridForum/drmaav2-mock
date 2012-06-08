DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o sqlite3.o persistence.o
TESTOBJS = test.o tests/test_app.o tests/test_dict.o tests/test_list.o tests/test_sessions.o tests/test_jsession.o tests/test_msession.o

CC = gcc
CFLAGS =

all: test setup


setup: setup_db.o $(DRMAAOBJS)
	$(CC) -o $@ setup_db.o $(DRMAAOBJS)

test: $(DRMAAOBJS) $(TESTOBJS)
	$(CC) -lcunit -o $@ $(DRMAAOBJS) $(TESTOBJS)

tests/%.o: tests/%.c tests/%.h $(DRMAAOBJS)
	$(CC) -c $< -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

drmaa2.o: drmaa2.c drmaa2.h drmaa2-mock.h drmaa2-dict.h drmaa2-list.h persistence.h
	$(CC) $(CFLAGS) -c $<

persistence.o: persistence.c persistence.h drmaa2-mock.h sqlite3.h drmaa2.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm setup
	rm -f tests/*.o test
