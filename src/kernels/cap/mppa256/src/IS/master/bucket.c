/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * bucket.c - Bucket library implementation.
 */

#include <assert.h>
#include <util.h>
#include "master.h"

/*
 * Creates a bucket.
 */
struct bucket *bucket_create(void)
{
	struct bucket *b;
	
	b = smalloc(sizeof(struct bucket));
	
	/* Initialize bucket. */
	b->size = 0;
	b->head = NULL;
	
	return (b);
}

/*
 * Destroys a bucket.
 */
void bucket_destroy(struct bucket *b)
{
	struct minibucket *minib;
	
	/* Destroy mini-buckets. */
	while (b->head != NULL)
	{
		minib = b->head;
		b->head = minib->next;
		minibucket_destroy(minib);
	}
	
	free(b);
}

/*
 * Merges a bucket.
 */
void bucket_merge(struct bucket *b, int *array)
{
	int i;                  /* Loop index.          */
	struct minibucket *w;   /* Working mini-bucket. */
	struct minibucket *min; /* Min mini-bucket.     */
	
	/* Merge buckets. */
	for (i = 0;  i < b->size; i++, array++)
	{
		min = b->head;
		for (w = b->head; w != NULL; w = w->next)
		{
			if (minibucket_top(min) > minibucket_top(w))
				min = w;
		}
			
		minibucket_pop(min, *array);
	}
}

/*
 * Insert an item into a bucket.
 */
void bucket_insert(struct bucket **b, int x)
{
	struct minibucket *minib;
	
	if ((*b)->head == NULL)
		(*b)->head = minibucket_create();
	
	minib = (*b)->head;
	
	/* Create a new mini-bucket. */
	if (minibucket_full(minib))
	{
		minib = minibucket_create();
		minib->next = (*b)->head;
		(*b)->head = minib;
	}
	
	(*b)->size++;
	minibucket_push(minib, x);
}

/*
 * Pops a mini-bucket from a bucket.
 */
struct minibucket *bucket_pop(struct bucket *b)
{
	struct minibucket *minib;
	
	assert(bucket_size(b) > 0);
	
	/* Pop mini-bucket. */
	minib = b->head;
	b->head = minib->next;
	b->size -= minib->size;
	
	return (minib);
}

/*
 * Pushes a mini-bucket onto a bucket.
 */
void bucket_push(struct bucket *b, struct minibucket *minib)
{
	minib->next = b->head;
	b->head = minib;
	b->size += minib->size;
}
