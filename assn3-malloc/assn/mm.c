/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <tgmath.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
/* Team name */
"zumar",
/* First member's full name */
"Zaara Syeda",
/* First member's email address */
"zaara.syeda@mail.utoronto.ca",
/* Second member's full name (leave blank if none) */
"Ghulam Umar",
/* Second member's email address (leave blank if none) */
"ghulam.umar@mail.utoronto.ca" };

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
 *************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<10)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

void* heap_listp = NULL;
void *free_list[15];

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void) {
	int i, j;
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) -1)
		return -1;
	PUT(heap_listp, 0);                         // alignment padding
	PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
	PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // epilogue header // what is epilogue header?
	heap_listp += DSIZE; //why is it +DSIZE if we added 2DSIZE?

	for (i = 0; i < 15; i++) {
		free_list[i] = NULL;
	}

	return 0;
}

static inline int find_index(int size) {
	int index = -1;

	if (size <= 32) {
		index = 0;
	} else if (size > 32 && size <= 64) {
		index = 1;
	} else if (size > 64 && size <= 128) {
		index = 2;
	} else if (size > 128 && size <= 256) {
		index = 3;
	} else if (size > 256 && size <= 512) {
		index = 4;
	} else if (size > 512 && size <= 1024) {
		index = 5;
	} else if (size > 1024 && size <= 2048) {
		index = 6;
	} else if (size > 2048 && size <= 4096) {
		index = 7;
	} else if (size > 4096 && size <= 8192) {
		index = 8;
	} else if (size > 8192 && size <= 16384) {
		index = 9;
	} else if (size > 16384 && size <= 32768) {
		index = 10;
	} else if (size > 32768 && size <= 65536) {
		index = 11;
	} else if (size > 65536 && size <= 131072) {
		index = 12;
	} else if (size > 131072 && size <= 262144) {
		index = 13;
	} else if (size > 262144) {
		index = 14;
	}

	return index;
}

static inline void remove_from_free(void *bp) {

	int index;
	void *prev;
	void *next;

	size_t size = GET_SIZE(HDRP(bp));
	index = find_index(size);
	prev = GET(bp+WSIZE);
	next = GET(bp);

	//only item in list
	if ((prev == NULL) && (next == NULL)) {
		free_list[index] = NULL;
	} //first item of list
	else if ((prev == NULL) && (next != NULL)) {
		PUT(next+DSIZE, NULL);
		free_list[index] = next;
		//last item of list
	} else if (prev != NULL && next == NULL) {
		PUT(prev+WSIZE, NULL);
	}
	//in middle of list
	else {
		PUT(prev+WSIZE, next);
		PUT(next+DSIZE, prev);
	}
	PUT(bp, NULL);
	PUT(bp+WSIZE, NULL);
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp) {
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	void *next = NEXT_BLKP(bp);
	void *prev = PREV_BLKP(bp);

	size_t size = GET_SIZE(HDRP(bp));

	if (prev_alloc && next_alloc) { /* Case 1 */
		return bp;
	} else if (prev_alloc && !next_alloc) { /* Case 2 */
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));

		remove_from_free(next);

		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
		return (bp);
	} else if (!prev_alloc && next_alloc) { /* Case 3 */
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));

		remove_from_free(prev);

		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		return (PREV_BLKP(bp));
	} else { /* Case 4 */
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));

		remove_from_free(prev);
		remove_from_free(next);

		PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));

		return (PREV_BLKP(bp));
	}
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words) {
	char *bp;
	size_t size;

	/* Allocate an even number of words to maintain alignments */
	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
	if ((bp = mem_sbrk(size)) == (void *) -1)
		return NULL;

	/* Initialize free block header/footer and the epilogue header */
	PUT(HDRP(bp), PACK(size, 0));                // free block header
	PUT(FTRP(bp), PACK(size, 0));                // free block footer
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

	return bp;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
//need to implement checking bigger size blocks if not found in current size
// look for bigger blocks and then split them
void * find_fit(size_t asize) {
	void *bp;
	int index;
	size_t curr_size;

	void *head;
	void *next;
	void *prev;

	index = find_index(asize);
	next = NULL;
	prev = NULL;

	head = free_list[index];
	if (head != NULL) {
		curr_size = GET_SIZE(head);

		while ((curr_size < asize) && head != NULL) {
			head = GET(head+WSIZE);
			if (head != NULL)
				curr_size = GET_SIZE(head);
		}
	}

	while (head == NULL && index <= 14) {
		index++;
		head = free_list[index];
		if (head != NULL)
			break;
	}
	if (head == NULL) {
		return NULL;
	} else {
		remove_from_free(head + WSIZE);
	}

	return head + WSIZE;
}

static inline void add_to_free(void *split) {
	int index;
	size_t size;
	size = GET_SIZE(HDRP(split));

	index = find_index(size);

	void *head;
	if (free_list[index] == NULL) {
		PUT(split, NULL);
		PUT(split+WSIZE, NULL);
		free_list[index] = HDRP(split);
	} else {
		head = free_list[index];
		PUT(head+DSIZE, HDRP(split));
		PUT(split, head);
		PUT(split+WSIZE, NULL);
		free_list[index] = HDRP(split);
	}
}/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/

//implement splitting
void place(void* bp, size_t asize) {
	void *split;
	split = bp;

	size_t bsize = GET_SIZE(HDRP(bp));

	if ((bsize - asize) >= 32) {
		split += asize;
		PUT(HDRP(split), PACK(bsize-asize,0));
		PUT(FTRP(split), PACK(bsize-asize,0));
		add_to_free( split);
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));

	} else {
		/* Get the current block size */
		PUT(HDRP(bp), PACK(bsize, 1));
		PUT(FTRP(bp), PACK(bsize, 1));
	}
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) {
	if (bp == NULL) {
		return;
	}

	size_t size = GET_SIZE(HDRP(bp));

	PUT(HDRP(bp), PACK(size,0));
	PUT(FTRP(bp), PACK(size,0));
	void* coal_bp;
	coal_bp = coalesce(bp);
	add_to_free( coal_bp);
}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size) {
	size_t asize; /* adjusted block size */
	size_t extendsize; /* amount to extend heap if no fit */
	char * bp;

	/* Ignore spurious requests */
	if (size == 0)
		return NULL;

	/* Adjust block size to include overhead and alignment reqs. */
	if (size <= DSIZE)
		asize = 2 * DSIZE;
	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

	/* Search the free list for a fit */
	if ((bp = find_fit(asize)) != NULL) {
		place(bp, asize);
		return bp;
	}

	/* No fit found. Get more memory and place the block */
	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
		return NULL;

	place(bp, asize);
	return bp;
}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) {
	/* If size == 0 then this is just free, and we return NULL. */
	if (size == 0) {
		mm_free(ptr);
		return NULL;
	}
	/* If oldptr is NULL, then this is just malloc. */
	if (ptr == NULL)
		return (mm_malloc(size));

	void *oldptr = ptr;
	void *newptr;
	size_t copySize, asize;

	void *new_free;
	void *coal_new_free;
	new_free = ptr;
	copySize = GET_SIZE(HDRP(oldptr));

	if (size <= DSIZE)
		asize = 2 * DSIZE;
	else
		asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

	int diff = (copySize - (asize));

	//if copySize is larger than asize and we can safely split
	if (diff >= 32) {
		new_free += asize;
		PUT(HDRP(new_free), PACK(diff,0));
		PUT(FTRP(new_free), PACK(diff,0));
		add_to_free( new_free);
		PUT(HDRP(ptr), PACK(asize, 1));
		PUT(FTRP(ptr), PACK(asize, 1));
		return ptr;
	//if copySize is larger than asize but we cannot split
	}else if(diff >= 0 && diff < 32){
		PUT(HDRP(ptr), PACK(copySize,1));
		PUT(FTRP(ptr), PACK(copySize,1));
		return ptr;
	//if new size is larger then old size
	}else {
		void *next_block, *prev_block, *temp;
		size_t next_size,prev_size;
		int total;
		if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) {
			next_block = NEXT_BLKP(ptr);
			next_size = GET_SIZE(HDRP(next_block));

			total = copySize + next_size;

			if(total > asize){
				// merge with next block
				remove_from_free(next_block);
				PUT(HDRP(ptr),PACK(total,1));
				PUT(FTRP(next_block),PACK(total,1));
				//return ptr;
				/*diff=total-asize;
				if(diff>=32){
					new_free += asize;
					PUT(HDRP(new_free),PACK(diff,0));
					PUT(FTRP(new_free),PACK(diff,0));
					add_to_free(new_free);
					PUT(HDRP(ptr),PACK(asize,1));
					PUT(FTRP(ptr),PACK(asize,1));
				}*/
				return ptr;
			}

		}else if(!GET_ALLOC(PREV_BLKP(ptr))){
			prev_block = PREV_BLKP(ptr);
			prev_size = GET_SIZE(HDRP(prev_block));
			
			total = copySize + prev_size;
			if(total > asize){
				remove_from_free(prev_block);
				PUT(HDRP(prev_block),PACK(total,1));
				PUT(FTRP(ptr),PACK(total,1));
				temp = mm_malloc(copySize);
				memcpy(temp, ptr,copySize);
				memcpy(prev_block,temp,copySize);
				mm_free(temp);
				return prev_block;
			}
		}else if(!GET_ALLOC(PREV_BLKP(ptr)) && !GET_ALLOC(PREV_BLKP(ptr))){
			prev_block = PREV_BLKP(ptr);
			next_block = NEXT_BLKP(ptr);
			prev_size = GET_SIZE(HDRP(prev_block));
			next_size = GET_SIZE(HDRP(next_block));
			total = copySize + prev_size + next_size;
			if(total > asize){
				remove_from_free(prev_block);
				remove_from_free(next_block);
				PUT(HDRP(prev_block),PACK(total,1));
				PUT(FTRP(next_block),PACK(total,1));
				temp = mm_malloc(copySize);
				memcpy(temp, ptr,copySize);
				memcpy(prev_block,temp,copySize);
				mm_free(temp);
				return prev_block;
			}
		}

		newptr = mm_malloc(size);
		if (newptr == NULL)
			return NULL;

		/* Copy the old data. */

		copySize = GET_SIZE(HDRP(oldptr));
		if (size < copySize)
			copySize = size;
		memcpy(newptr, oldptr, copySize);
		mm_free(oldptr);
		return newptr;
	}
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void) {

	int i;
	void *ptr;

	//is every free block in the free list marked as free
	for(i=0;i<15;i++){
		ptr=free_list[i];
		while(ptr!=NULL){
			if(GET_ALLOC(ptr)){
				return 0;
			}
			ptr = GET(ptr+WSIZE);
		}
	}


	//contiguos free blocks that escaped coalescing
    	void *bp;
	void *next;
    	for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
		next = NEXT_BLKP(bp);
		if(GET_SIZE(HDRP(next))>0){
        		if (!GET_ALLOC(HDRP(bp)) && !GET_ALLOC(HDRP(next))){
				return 0;
			}
        	}
    	}


	int index=0;
	//is every free block in the free list
	for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
		if(!GET_ALLOC(HDRP(bp))){
        		index = find_index(GET_SIZE(HDRP(bp)));
			for(i=index;i<15;i++){
				ptr = free_list[index];
				while(ptr!=NULL){
					if(ptr==HDRP(bp)){
						i=15;
						break;
					}else{
						ptr=GET(ptr+WSIZE);
					}
				}
			}
			if(ptr==NULL)
				return 0;
		}
    	}

	//is size in header and footer same
        for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
		if(GET_SIZE(HDRP(bp))!=GET_SIZE(FTRP(bp))){
			return 0;
		}
	}		
	return 1;
}
