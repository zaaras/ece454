CC = g++-4.7
CFLAGS = -O3 -pthread

all: randtrack randtrack_tm randtrack_global_lock randtrack_list_lock randtrack_element_lock randtrack_reduction 

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS)  randtrack.cc -o randtrack

randtrack_tm: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) -fgnu-tm -DTM  randtrack.cc -o randtrack_tm


randtrack_global_lock: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) -DGL  randtrack.cc -o randtrack_global_lock

randtrack_list_lock: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) -DLLL  randtrack.cc -o randtrack_list_lock

randtrack_element_lock: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) -DELL  randtrack.cc -o randtrack_element_lock

randtrack_reduction: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) -DRED  randtrack.cc -o randtrack_reduction


clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock  randtrack_element_lock randtrack_reduction
