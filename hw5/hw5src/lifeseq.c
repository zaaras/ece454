/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <pthread.h>

/*
 *We implemented parallelization using 4 threads.
 *We also removed all the if statements inside the loops by breaking up the board calculations.
 * We first calculate row0, then the last row, then col0, then the last column, and finally calculate the inner board.
 */


/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
typedef struct thread_args_struct {
	char *inboard;
	char *outboard;
	int start_row;
	int end_row;
	int ncols;
	int nrows;
} thread_args;

#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define TCOUNT 4

#define BOARD( __board, __i, __j )  (__board[LDA*(__i) + (__j)])

//This function creates 4 threads to run the parallel game of life function, waits for all threads to return and swaps the board.
char*
sequential_game_of_life(char* outboard, char* inboard, const int nrows,
		const int ncols, const int gens_max) {

	/* HINT: in the parallel decomposition, LDA may not be equal to
	 nrows! */
	const int LDA = nrows;
	int curgen, i, inc;

	inc = nrows / TCOUNT;

	pthread_t *thrd[TCOUNT];
	thread_args *args[TCOUNT];

	for (curgen = 0; curgen < gens_max; curgen++) {
		for (i = 0; i < TCOUNT; i++) {
			args[i] = (thread_args*) malloc(sizeof(thread_args));
			args[i]->inboard = inboard;
			args[i]->outboard = outboard;
			args[i]->start_row = i * inc;
			args[i]->end_row = i * inc + inc;
			args[i]->ncols = ncols;
			args[i]->nrows = nrows;
			pthread_create(&thrd[i], NULL, &parallel_game_of_life,
					(void *) args[i]);
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

//This function runs the game_of_life function by breaking the board into chunks.
// Each thread is given the start_row and end_row argument to calcualte the output for.
void parallel_game_of_life(void *args) {
	thread_args *t_args = args;
	char *inboard = t_args->inboard;
	char *outboard = t_args->outboard;
	int start_row = t_args->start_row;
	int end_row = t_args->end_row;
	int ncols = t_args->ncols;
	int nrows = t_args->nrows;

	int i, j, endr, endc, temp;

	int inorth, isouth, jwest, jeast;
	char neighbor_count;
	const int LDA = nrows;

	if (end_row > nrows) {
		end_row = nrows;
	}
	endr = end_row;
	if (endr == nrows) {
		endr--;
	}
	endc = ncols - 1;

	//output for row0
	if (start_row == 0) {
		i = start_row;
		inorth = nrows - 1;
		isouth = i + 1;

		j = 0;
		jwest = endc;
		jeast = j + 1;

		neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard,inorth, j)
		+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
		+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
		+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

		BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));

		j = ncols - 1;
		jwest = j - 1;
		jeast = 0;

		neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard,inorth, j)
		+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
		+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
		+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

		BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));

		temp = ncols-1;
		for (j = 1; j < temp; j++) {
			jwest = j - 1;
			jeast = j + 1;

			neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard,inorth, j)
				+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
				+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
				+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}

	i = start_row;
	if (start_row == 0) {
		i++;
	}

	//output for inner board
	for (; i < endr; i++) {
		inorth = i - 1;
		isouth = i + 1;
		for (j = 1; j < endc; j++) {
			jwest = j - 1;
			jeast = j + 1;

			neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard,inorth, j)
			+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
			+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
			+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}

	//output for last row
	if (end_row == nrows) {
		i = end_row - 1;
		inorth = i - 1;
		isouth = 0;

		j = 0;
		jwest = endc;
		jeast = j + 1;
		neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard,inorth, j)
		+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
		+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
		+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

		BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));

		j = ncols - 1;
		jwest = j - 1;
		jeast = 0;
		neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard,inorth, j)
		+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
		+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
		+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

		BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));
		temp = ncols-1;
		for (j = 1; j < temp; j++) {
			jwest = j - 1;
			jeast = j + 1;

			neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard, inorth, j)
			+ BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
			+ BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
			+ BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}

	if (start_row == 0) {
		i = start_row + 1;
	} else {
		i = start_row;
	}
	if (end_row == nrows) {
		endr = end_row - 1;
	} else {
		endr = end_row;
	}

	int j1, j1west, j1east;
	int j2, j2west, j2east;

	j1 = 0;
	j1west = ncols - 1;
	j1east = j1 + 1;

	j2 = ncols - 1;
	j2east = 0;
	j2west = j2 - 1;

	//output for first and last column
	for (; i < endr; i++) {
		inorth = i - 1;
		isouth = i + 1;
		neighbor_count = BOARD (inboard, inorth, j1west)+BOARD (inboard, inorth, j1)
		+ BOARD (inboard, inorth, j1east) + BOARD (inboard, i, j1west)
		+ BOARD (inboard, i, j1east) + BOARD (inboard, isouth, j1west)
		+ BOARD (inboard, isouth, j1) + BOARD (inboard, isouth, j1east);

		BOARD(outboard, i, j1)= alivep (neighbor_count, BOARD (inboard, i, j1));

		neighbor_count = BOARD (inboard, inorth, j2west)+BOARD (inboard, inorth, j2)
		+ BOARD (inboard, inorth, j2east) + BOARD (inboard, i, j2west)
		+ BOARD (inboard, i, j2east) + BOARD (inboard, isouth, j2west)
		+ BOARD (inboard, isouth, j2) + BOARD (inboard, isouth, j2east);

		BOARD(outboard, i, j2)= alivep (neighbor_count, BOARD (inboard, i, j2));
	}

	/*j = ncols - 1;
	 jeast = 0;
	 jwest = j - 1;
	 if (start_row == 0) {
	 i = start_row + 1;
	 } else {
	 i = start_row;
	 }
	 if (end_row == nrows) {
	 endr = end_row - 1;
	 } else {
	 endr = end_row;
	 }
	 for (; i < endr; i++) {
	 inorth = i - 1;
	 isouth = i + 1;

	 neighbor_count = BOARD (inboard, inorth, jwest)+BOARD (inboard, inorth, j)
	 + BOARD (inboard, inorth, jeast) + BOARD (inboard, i, jwest)
	 + BOARD (inboard, i, jeast) + BOARD (inboard, isouth, jwest)
	 + BOARD (inboard, isouth, j) + BOARD (inboard, isouth, jeast);

	 BOARD(outboard, i, j)= alivep (neighbor_count, BOARD (inboard, i, j));

	 }*/
}
