/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MASTER_H_
#define MASTER_H_

	#include <arch.h>

/*============================================================================*
 *                             Mini-Buckets Library                           *
 *============================================================================*/
 
	/*
	 * Size of mini-bucket.
	 */
	#define MINIBUCKET_SIZE 262144
 
	/*
	 * Mini-bucket.
	 */
	struct minibucket
	{
		int size;                      /* Current size.               */
		int elements[MINIBUCKET_SIZE]; /* Elements.                   */
		struct minibucket *next;       /* Next mini-bucket in a list. */
	};

	/*
	 * Creates a mini-bucket.
	 */
	extern struct minibucket *minibucket_create(void);
	
	/*
	 * Destroys a mini bucket.
	 */
	extern void minibucket_destroy(struct minibucket *minib);

	/*
	 * Asserts if a mini-bucket is empty.
	 */
	#define minibucket_empty(minib) \
		((minib)->size == 0)

	/*
	 * Asserts if a bucket is full.
	 */
	#define minibucket_full(minib) \
		((minib)->size == MINIBUCKET_SIZE)

	/*
	 * Pushes an item onto a mini-bucket.
	 */
	#define minibucket_push(minib, x) \
		((minib)->elements[(minib)->size++] = (x))

	/*
	 * Pops an item from a mini-bucket.
	 */
	#define minibucket_pop(minib, x) \
		((x) = (minib)->elements[--(minib)->size])

	/*
	 * Returns the top element in a bucket.
	 */
	#define minibucket_top(minib) \
		((minib)->elements[(minib)->size - 1])

/*============================================================================*
 *                                Buckets Library                             *
 *============================================================================*/
 	
	/*
	 * Bucket.
	 */
	struct bucket
	{
		int size;                /* Number of elements.  */
		struct minibucket *head; /* List of mini-buckets.*/
	};

	/*
	 * Creates a bucket.
	 */
	extern struct bucket *bucket_create(void);
	
	/*
	 * Destroys a bucket.
	 */
	extern void bucket_destroy(struct bucket *b);
	
	/*
	 * Merges a bucket.
	 */
	extern void bucket_merge(struct bucket *b, int *array);
	
	/*
	 * Pops a mini-bucket from a bucket.
	 */
	extern struct minibucket *bucket_pop(struct bucket *b);
	
	/*
	 * Pushes a mini-bucket onto a bucket.
	 */
	extern void bucket_push(struct bucket *b, struct minibucket *minib);
	
	/*
	 * Insert an item into a bucket.
	 */
	extern void bucket_insert(struct bucket **b, int x);
	
	/*
	 * Returns the size of a bucket.
	 */
	#define bucket_size(b) \
		((b)->size)


/*============================================================================*
 *                    Inter Process Communication Library                     *
 *============================================================================*/
		
	/*
	 * Close NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/*
	 * Joins slave processes.
	 */
	extern void join_slaves(void);
	
	/*
	 * Kills slave processes.
	 */
	extern void kill_slaves(void);
	
	/*
	 * Open NoC connectors.
	 */
	extern void open_noc_connectors(void);

	/*
	 * Spawns slave processes.
	 */
	extern void spawn_slaves(void);
	
	/*
	 * Waits for slaves to be ready.
	 */
	extern void sync_slaves(void);
	
	
	/* Channels. */
	extern int infd[NUM_CLUSTERS];  /* Input channels.  */
	extern int outfd[NUM_CLUSTERS]; /* Output channels. */

#endif /* MASTER_H_ */
