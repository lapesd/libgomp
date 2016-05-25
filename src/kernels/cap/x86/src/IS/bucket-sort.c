/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <limits.h>
#include <omp.h>
#include <util.h>
#include "is.h"
#include <stdio.h>

/* Number of buckets. */
#define NUM_BUCKETS 8192

/*
 * Bucket sort algorithm.
 */
void integer_sort(int *array, int n)
{
	int max, _max;           /* Max number in array. */
	int range;               /* Bucket range.        */
	int i, j, k;             /* Loop indexes.        */
	int *indexes;            /* Index for buckets.   */
	struct darray **buckets; /* Buckets.            */

	indexes = smalloc(NUM_BUCKETS*sizeof(int));

	/* Create buckets. */
	buckets = smalloc(NUM_BUCKETS*sizeof(struct darray *));
	for (i = 0; i < NUM_BUCKETS; i++)
		buckets[i] = darray_create(n/NUM_BUCKETS);
	
	max = INT_MIN;
	
	#pragma omp parallel private(i, j, k, _max)
	{	
		_max = INT_MIN;

		/* Find max number in the array. */
		#pragma omp for schedule(static)
		for (i = 0; i < n; i++)
		{
			/* Found. */
			if (array[i] > _max)
				_max = array[i];
		}

		#pragma omp critical
		{
			if (_max > max) {
				max = _max;
			}
		}
		
		#pragma omp master
		range = max/NUM_BUCKETS;
		#pragma omp barrier
		
		/* Distribute numbers into buckets. */
		#pragma omp master
		for (i = 0; i < n; i++)
		{
			j = array[i]/range;
			if (j >= NUM_BUCKETS)
				j = NUM_BUCKETS - 1;

			darray_append(buckets[j], array[i]);
		}
		
		/* Sort Each bucket. */
		#pragma omp for schedule(dynamic)
		for (i = 0; i < NUM_BUCKETS; i++)
		{
			if (darray_size(buckets[i]) > 0)
				sort(buckets[i]);
		}
		
		#pragma omp master
		{
			/* Build indexes. */
			indexes[0] = 0;
			for (i = 1; i < NUM_BUCKETS; i++)
				indexes[i] = indexes[i - 1] + darray_size(buckets[i]);
		
			/* Rebuild array. */
			for (i = 0; i < NUM_BUCKETS; i++)
			{
				k = indexes[i];
					
				for (j = 0; j < darray_size(buckets[i]); j++)
					array[k + j] = darray_get(buckets[i], j);
			}
		}
	}
	
	/* House keeping. */
	for (i = 0; i < NUM_BUCKETS; i++)
		darray_destroy(buckets[i]);
	free(buckets);
	free(indexes);
}
