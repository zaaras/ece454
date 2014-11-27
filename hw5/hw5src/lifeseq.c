/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <pthread.h>
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
typedef struct thread_args_struct{
	char *inboard;
	char *outboard;
	int start_row;
	int end_row;
	int ncols;
	int nrows;
}thread_args;

#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define TCOUNT 4

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

char*
sequential_game_of_life(char* outboard, char* inboard, const int nrows,
		const int ncols, const int gens_max) {
	/* HINT: in the parallel decomposition, LDA may not be equal to
	 nrows! */
	const int LDA = nrows;
	int curgen, i, j, inc;

	inc = nrows / TCOUNT;

	pthread_t *thrd[TCOUNT];
	thread_args *args[TCOUNT];

	for (curgen = 0; curgen < gens_max; curgen++) {
		for (i = 0; i < TCOUNT; i++) {
			args[i] =  (thread_args*)malloc(sizeof(thread_args));
			args[i]->inboard=inboard;
			args[i]->outboard=outboard;
			args[i]->start_row=i*inc;
			args[i]->end_row=i*inc+inc;
			args[i]->ncols=ncols;
			args[i]->nrows=nrows;
			pthread_create(&thrd[i], NULL, &parallel_game_of_life, (void *)args[i]);
		}

		for (i = 0; i < TCOUNT; i++) {
			pthread_join(thrd[i], NULL);
		}
		/* HINT: you'll be parallelizing these loop(s) by doing a
		 geometric decomposition of the output */
		SWAP_BOARDS( outboard, inboard);

	}
	/*
	 * We return the output board, so that we know which one contains
	 * the final result (because we've been swapping boards around).
	 * Just be careful when you free() the two boards, so that you don't
	 * free the same one twice!!!
	 */
	return inboard;
}

void parallel_game_of_life(void *args) {
	thread_args *t_args = args;
	char *inboard = t_args->inboard;
	char *outboard = t_args->outboard;
	int start_row = t_args->start_row;
	int end_row = t_args->end_row;
	int ncols = t_args->ncols;
	int nrows = t_args->nrows;

	int i,j;

	const int LDA = nrows;
	for (i = start_row; i < end_row; i++) {
		//tiling
		//loop unrolling
		for (j = 0; j < ncols; j++) {
			const int inorth = mod(i - 1, nrows);
			const int isouth = mod(i + 1, nrows);
			const int jwest = mod(j - 1, ncols);
			const int jeast = mod(j + 1, ncols);

			const char neighbor_count = BOARD (inboard, inorth, jwest)+
			BOARD (inboard, inorth, j) +
			BOARD (inboard, inorth, jeast) +
			BOARD (inboard, i, jwest) +
			BOARD (inboard, i, jeast) +
			BOARD (inboard, isouth, jwest) +
			BOARD (inboard, isouth, j) +
			BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));

		}
	}

}
