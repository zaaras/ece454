
CC = g++-4.7
CFLAGS = -O3 -pthread

all: randtrack 

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack

randtrack_tm: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) -fgnu-tm -DTM  randtrack.cc -o randtrack_tm



clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock
