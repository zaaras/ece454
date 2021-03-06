
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/*
 * ECE454 Students:
 * Please fill in the following team struct
 */
team_t team = {
	"zZzumar",                  /* Team name */

	"Zaara Syeda",                    /* First member full name */
	"998199765",                 /* First member student number */
	"zaara.syeda@mail.utoronto.ca",                 /* First member email address */

	"Ghulam Umar",                           /* Second member full name */
	"998513988",                           /* Second member student number */
	"ghulam.umar@mail.utoronto.ca"                            /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

#ifdef GL
pthread_mutex_t lock;
#endif

class sample;

class sample {
	unsigned my_key;
	public:
	sample *next;
	unsigned count;
#ifdef ELL
	pthread_mutex_t lock;
	sample(unsigned the_key){my_key = the_key; count = 0; pthread_mutex_init(&lock,NULL);};
	void add(){
		pthread_mutex_lock(&lock);
		count++;
		pthread_mutex_unlock(&lock);
	}
#endif
#ifndef ELL
	sample(unsigned the_key){my_key = the_key; count = 0;};
#endif
	sample(const sample &other){my_key = other.my_key; count = other.count; next = other.next;};
	sample(const sample *other){my_key = other->my_key; count = other->count; next = other->next;};
	sample(){my_key = 0; count = 0; next = NULL;};
	unsigned key(){return my_key;}
	void print(FILE *f){printf("%d %d\n",my_key,count);}

};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".
hash<sample,unsigned> h;
hash<sample,unsigned> hash_array[4];

void *oneThreads(void*){
	int i,j,k;
	int rnum;
	unsigned key;
	sample *s;

#ifdef RED
	for (i=0; i<4; i++){

		rnum=i;


		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			if (!(s = hash_array[i].lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				hash_array[i].insert(s);
			}

			// increment the count for the sample
			s->count++;

		}
	}
#endif


#ifdef ELL

	for (i=0; i<4; i++){
		rnum=i;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			//h.lockList(key);
			//h.readLockList(key);

			if (!(s = h.lookup(key))){

				h.lockList(key);
				//h.readUnlockList(key);
				//h.upgradeLock(key);
				// insert a new element for it into the hash table
				//if (!(s = h.lookup(key))){

					//s = new sample(key);
					//h.insert(s);
				s = h.lookupInsert(key);
				//}

				h.unlockList(key);
			}
			s->add();

		}
	}
#endif

#ifdef LLL
	for (i=0; i<4; i++){
		rnum=i;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			h.lockList(key);
			if (!(s = h.lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
			}

			// increment the count for the sample
			s->count++;
			h.unlockList(key);

		}
	}
#endif

#ifdef GL
	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
	}
	// process streams starting with different initial numbers
	for (i=0; i<4; i++){
		rnum=i;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			pthread_mutex_lock(&lock);
			if (!(s = h.lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
			}

			// increment the count for the sample
			s->count++;
			pthread_mutex_unlock(&lock);

		}
	}
	pthread_mutex_destroy(&lock);
#endif

#ifdef TM

	// process streams starting with different initial numbers
	for (i=0; i<4; i++){
		rnum=i;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			__transaction_atomic {
				if (!(s = h.lookup(key))){

					// insert a new element for it into the hash table
					s = new sample(key);
					h.insert(s);
				}

				// increment the count for the sample
				s->count++;
			}
		}
	}
#endif

}


void *twoThreads(void* seed){
	int i,j,k;
	int rnum;
	unsigned key;
	sample *s;
	int *tmp;
	tmp = new int(*(int *)seed);
	(*tmp)--;

#ifdef RED
	for (i=0; i<2; i++){
		(*tmp)++;
		rnum=*tmp;


		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			if (!(s = hash_array[*tmp].lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				hash_array[*tmp].insert(s);
			}

			// increment the count for the sample
			s->count++;

		}
	}
#endif


#ifdef ELL

	for (i=0; i<2; i++){
		(*tmp)++;
		rnum=*tmp;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			//h.lockList(key);
			//h.readLockList(key);

			if (!(s = h.lookup(key))){

				h.lockList(key);
				//h.readUnlockList(key);
				//h.upgradeLock(key);
				// insert a new element for it into the hash table
				//if (!(s = h.lookup(key))){

					//s = new sample(key);
					//h.insert(s);
				s = h.lookupInsert(key);
				//}

				h.unlockList(key);
			}
			s->add();

		}
	}
#endif

#ifdef LLL
	for (i=0; i<2; i++){
		(*tmp)++;
		rnum=*tmp;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			h.lockList(key);
			if (!(s = h.lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
			}

			// increment the count for the sample
			s->count++;
			h.unlockList(key);

		}
	}
#endif

#ifdef GL
	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
	}
	// process streams starting with different initial numbers
	for (i=0; i<2; i++){
		(*tmp)++;
		rnum=*tmp;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			pthread_mutex_lock(&lock);
			if (!(s = h.lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
			}

			// increment the count for the sample
			s->count++;
			pthread_mutex_unlock(&lock);

		}
	}
	pthread_mutex_destroy(&lock);
#endif

#ifdef TM

	// process streams starting with different initial numbers
	for (i=0; i<2; i++){
		(*tmp)++;
		rnum=*tmp;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			__transaction_atomic {
				if (!(s = h.lookup(key))){

					// insert a new element for it into the hash table
					s = new sample(key);
					h.insert(s);
				}

				// increment the count for the sample
				s->count++;
			}
		}
	}
#endif

}


void *four_threads(void* seed){
	int i,j,k,key;
	sample *s;
	int rnum;
	rnum = *((int *)seed);
	int thread_num = rnum;

#ifdef RED
	for (j=0; j<SAMPLES_TO_COLLECT; j++){

		// skip a number of samples
		for (k=0; k<samples_to_skip; k++){
			rnum = rand_r((unsigned int*)&rnum);
		}

		// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
		key = rnum % RAND_NUM_UPPER_BOUND;

		// if this sample has not been counted before
		if (!(s = hash_array[thread_num].lookup(key))){

			// insert a new element for it into the hash table
			s = new sample(key);
			hash_array[thread_num].insert(s);
		}

		// increment the count for the sample
		s->count++;

	}
#endif

#ifdef ELL

		int tmp;
		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			// if this sample has not been counted before
			if (!(s = h.lookup(key))){
				// all threads need to insert key
				// s count needs to be 0
				// if another thread is here the lock for key is taken all other threads need to leave
				// thread also know the key it wants to insert
				// knows key is not in any list
				// knows the list the key belongs in

				h.lockList(key);

				// insert a new element for it into the hash table
				//if (!(s = h.lookup(key))){

					//s = new sample(key);
					//h.insert(s);
				s = h.lookupInsert(key);
				//}

				h.unlockList(key);
			}

			s->add();
		}

#endif

	//printf("%d\n", rnum);
#ifdef LLL
	// collect a number of samples
	for (j=0; j<SAMPLES_TO_COLLECT; j++){

		// skip a number of samples
		for (k=0; k<samples_to_skip; k++){
			rnum = rand_r((unsigned int*)&rnum);
		}

		// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
		key = rnum % RAND_NUM_UPPER_BOUND;

		// if this sample has not been counted before

		h.lockList(key);
		if (!(s = h.lookup(key))){

			// insert a new element for it into the hash table
			s = new sample(key);
			h.insert(s);
		}

		// increment the count for the sample
		s->count++;
		h.unlockList(key);

	}
#endif

#ifdef GL
	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
	}

	// collect a number of samples
	for (j=0; j<SAMPLES_TO_COLLECT; j++){

		// skip a number of samples
		for (k=0; k<samples_to_skip; k++){
			rnum = rand_r((unsigned int*)&rnum);
		}

		// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
		key = rnum % RAND_NUM_UPPER_BOUND;

		// if this sample has not been counted before
		pthread_mutex_lock(&lock);
		if (!(s = h.lookup(key))){

			// insert a new element for it into the hash table
			s = new sample(key);
			h.insert(s);
		}

		// increment the count for the sample
		s->count++;
		pthread_mutex_unlock(&lock);

	}

	pthread_mutex_destroy(&lock);
#endif

#ifdef TM

	// collect a number of samples
	for (j=0; j<SAMPLES_TO_COLLECT; j++){

		// skip a number of samples
		for (k=0; k<samples_to_skip; k++){
			rnum = rand_r((unsigned int*)&rnum);
		}

		// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
		key = rnum % RAND_NUM_UPPER_BOUND;

		// if this sample has not been counted before
		__transaction_atomic {
			if (!(s = h.lookup(key))){

				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
			}

			// increment the count for the sample
			s->count++;
		}
	}
#endif
}

void combine_and_print(){
	hash<sample, unsigned> final;
	final.setup(14);

	sample *s;
	sample *f;

	int i,j;
	for (i = 0; i < RAND_NUM_UPPER_BOUND; i++){
		for(j=0;j<4;j++){
			if(s=hash_array[j].lookup(i)){
				if(!(f=final.lookup(i))){
					f = new sample(i);
					final.insert(f);
				}
				f->count += s->count;
			}
		}
	}
	final.print();
}

int main (int argc, char* argv[]){
	int i,j,k;
	int rnum;
	unsigned key;
	sample *s;
	// Print out team information
	printf( "Team Name: %s\n", team.team );
	printf( "\n" );
	printf( "Student 1 Name: %s\n", team.name1 );
	printf( "Student 1 Student Number: %s\n", team.number1 );
	printf( "Student 1 Email: %s\n", team.email1 );
	printf( "\n" );
	printf( "Student 2 Name: %s\n", team.name2 );
	printf( "Student 2 Student Number: %s\n", team.number2 );
	printf( "Student 2 Email: %s\n", team.email2 );
	printf( "\n" );

	// Parse program arguments
	if (argc != 3){
		printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
		exit(1);
	}
	sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
	sscanf(argv[2], " %d", &samples_to_skip);

	// initialize a 16K-entry (2**14) hash of empty lists
	h.setup(14);
	for(i=0;i<4;i++){
		hash_array[i].setup(14);
	}

	pthread_t *thrd;
	int *tmp;

	if(num_threads==4){
		thrd = new pthread_t[4];

		for(i=0;i<4;i++){
			tmp = new int;
			*tmp = i;
			pthread_create(&thrd[i],NULL,&four_threads,(void *)tmp);
		}

		for( i=0; i<4; i++ ){
			pthread_join(thrd[i], NULL);
		}
	}

	int arg=0;
	if(num_threads==2){

		thrd = new pthread_t[2];

		for(i=0;i<2;i++){
			tmp = new int;
			*tmp = i*2;
			pthread_create(&thrd[i],NULL,&twoThreads,(void *)tmp);
		}

		for( i=0; i<2; i++ ){
			pthread_join(thrd[i], NULL);
		}
	}

	// process streams starting with different initial numbers
	if(num_threads==1){

		pthread_t tmpT;

		pthread_create(&tmpT,NULL,&oneThreads,NULL);

		pthread_join(tmpT, NULL);

		/*for (i=0; i<NUM_SEED_STREAMS; i++){
			rnum = i;

			// collect a number of samples
			for (j=0; j<SAMPLES_TO_COLLECT; j++){

				// skip a number of samples
				for (k=0; k<samples_to_skip; k++){
					rnum = rand_r((unsigned int*)&rnum);
				}

				// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
				key = rnum % RAND_NUM_UPPER_BOUND;

				// if this sample has not been counted before
				if (!(s = h.lookup(key))){

					// insert a new element for it into the hash table
					s = new sample(key);
					h.insert(s);
				}

				// increment the count for the sample
				s->count++;
			}
		}*/

	}

	#ifdef RED
//		if(num_threads!=1)
			combine_and_print();
//		else
//			h.print();
	#endif
	// print a list of the frequency of all samples
	#ifndef RED
		h.print();
	#endif

}


