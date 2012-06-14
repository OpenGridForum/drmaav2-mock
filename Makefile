DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o lib/sqlite3.o persistence.o
TESTOBJS = test.o tests/test_app.o tests/test_dict.o tests/test_list.o tests/test_sessions.o tests/test_jsession.o tests/test_msession.o

CC = gcc
CFLAGS =

all: test setup wrapper 

local: large test_app

test_app: test_app.o $(DRMAAOBJS)
	$(CC) -o $@ test_app.o $(DRMAAOBJS)

large: large.o $(DRMAAOBJS)
	$(CC) -o $@ large.o $(DRMAAOBJS)


wrapper: wrapper.o persistence.o lib/sqlite3.o drmaa2-list.o
	$(CC) $< lib/sqlite3.o persistence.o drmaa2-list.o -o $@

setup: setup_db.o $(DRMAAOBJS)
	$(CC) -o $@ setup_db.o $(DRMAAOBJS)

test: $(DRMAAOBJS) $(TESTOBJS) wrapper.o
	$(CC) -lcunit -o $@ $(DRMAAOBJS) $(TESTOBJS)

tests/%.o: tests/%.c tests/%.h $(DRMAAOBJS)
	$(CC) -c $< -o $@

lib/%.o: lib/%.c lib/%.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

drmaa2.o: drmaa2.c drmaa2.h drmaa2-mock.h drmaa2-dict.h drmaa2-list.h persistence.h drmaa2-debug.h
	$(CC) $(CFLAGS) -c $<

persistence.o: persistence.c persistence.h drmaa2-mock.h drmaa2.h drmaa2-debug.h
	$(CC) $(CFLAGS) -c $<

wrapper.o: wrapper.c drmaa2-debug.h
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f *.o
	rm -f setup wrapper
	rm -f tests/*.o test
	rm -f lib/*.o
	rm -f app large test_app
