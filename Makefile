DRMAAOBJS = drmaa2.o drmaa2-list.o drmaa2-dict.o sqlite3.o persistence.o
TESTOBJS = test.o tests/test_app.o tests/test_dict.o tests/test_list.o tests/test_sessions.o tests/test_jsession.o tests/test_msession.o

CC = gcc
CFLAGS =

all: test app setup


app: $(DRMAAOBJS) app.o
	$(CC) -o $@ $(DRMAAOBJS) app.o

setup: sqlite3.o setup_db.o
	$(CC) -o $@ sqlite3.o setup_db.o


test: $(DRMAAOBJS) $(TESTOBJS)
	$(CC) -lcunit -o $@ $(DRMAAOBJS) $(TESTOBJS)

tests/%.o: tests/%.c tests/%.h $(DRMAAOBJS)
	$(CC) -c $< -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

drmaa2.o: drmaa2.c drmaa2.h drmaa2-dict.o drmaa2-list.o sqlite3.o
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm setup app
	rm -f tests/*.o test
