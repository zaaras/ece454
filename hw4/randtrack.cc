#include<pthread.h>
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
	"Team Name",                  /* Team name */

	"AAA BBB",                    /* First member full name */
	"9999999999",                 /* First member student number */
	"AAABBB@CCC",                 /* First member email address */

	"",                           /* Second member full name */
	"",                           /* Second member student number */
	""                            /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

pthread_mutex_t lock;

class sample;

class sample {
	unsigned my_key;
	public:
	sample *next;
	unsigned count;

	sample(unsigned the_key){my_key = the_key; count = 0;};
	unsigned key(){return my_key;}
	void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> h;

void *twoThreads(void* seed){
	int i,j,k;
	int rnum;
	unsigned key;
	sample *s;

	rnum = *((int *)seed)-1;

#ifdef GL
	if (pthread_mutex_init(&lock, NULL) != 0){
    		printf("\n mutex init failed\n");
		}
	// process streams starting with different initial numbers
		for (i=0; i<(NUM_SEED_STREAMS/2); i++){
			rnum++;

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
	for (i=0; i<(NUM_SEED_STREAMS/2); i++){
		rnum++;

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

	pthread_t *thrd;

	if(num_threads==4){
		thrd = new pthread_t[4];
		for(i=0;i<4;i++){
			pthread_create(&thrd[i],NULL,&four_threads,(void *)&i);

			pthread_join(thrd[i],NULL);
		}

		for( i=0; i<4; i++ ){
 	 		pthread_join(thrd[i], NULL);
 		}
	}


	if(num_threads==2){
		thrd = new pthread_t[2];
		for(i=0;i<2;i++){
			pthread_create(&thrd[i],NULL,&twoThreads,(void *)&i);

			pthread_join(thrd[i],NULL);
		}
	}

	// process streams starting with different initial numbers
	if(num_threads==1){
		for (i=0; i<NUM_SEED_STREAMS; i++){
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
		}

	}
	// print a list of the frequency of all samples
	h.print();

}


