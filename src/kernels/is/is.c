/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
 
#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#include <profile.h>
#include <darray.h>
#include <util.h>
#include <is.h>

#define MAX_THREADS 16

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

/**
 * @brief Number of buckets.
 * 
 * @details Number of buckets. Adjust this to improve cache locality.
 */
#define NR_BUCKETS IS_NCLASSES

/*
 * Exchange two numbers.
 */
#define exch(a, b, t) \
	{ (t) = (a); (a) = (b); (b) = (t); }

/*
 * Compare and exchange two numbers.
 */
#define compexgh(a, b, t)    \
	if ((b) < (a))           \
		exch((b), (a), (t)); \
		
long max (int *a, long n, long i, long j, long k) {
    long m = i;
    
    if ((j < n) && (a[j] > a[m]))
        m = j;
    if ((k < n) && (a[k] > a[m]))
        m = k;
    
    return m;
}
 
void downheap (int *a, long n, int i) {
	int t;
    while (1) {
        long j = max(a, n, i, 2 * i + 1, 2 * i + 2);
        if (j == i) {
            break;
        }
        exch(a[i], a[j], t);
        i = j;
    }
}
 
void heapsort (int *a, long n)
{
	int t;
	
    for (long i = (n - 2) / 2; i >= 0; i--) {
        downheap(a, n, i);
    }
    for (long i = 0; i < n; i++) {
		exch(a[0], a[n - i - 1], t)
        downheap(a, n - i - 1, 0);
    }
}

/*
 * Sorts an array of numbers.
 */
void is(struct darray *da)
{
	heapsort(da->elements, da->size);
}

/**
 * @brief integer sort kernel.
 */
void integer_sort(int *numbers, long nnumbers)
{
	int min, max;            /* Max and min numbers.   */
	int range;               /* Bucket range.          */
	long *indexes;           /* Index for buckets.     */
	struct darray **buckets; /* Buckets.               */
#if defined(_SCHEDULE_SRR_)
	unsigned _buckets_size[NR_BUCKETS];
#endif
	long workload[MAX_THREADS];
	memset(workload, 0, sizeof(workload));

	indexes = smalloc(NR_BUCKETS*sizeof(long));

	/* Create buckets. */
	buckets = smalloc(NR_BUCKETS*sizeof(struct darray *));
	for (long i = 0; i < NR_BUCKETS; i++)
		buckets[i] = darray_create(nnumbers/NR_BUCKETS);
	
	/* Find max number in the array. */
	max = INT_MIN; min = INT_MAX;
	#pragma omp parallel for reduction(min:min) reduction(max:max)
	for (long i = 0; i < nnumbers; i++)
	{
		/* Found max. */
		if (numbers[i] > max)
			max = numbers[i];
		
		/* Found min. */
		else if (numbers[i] < min)
			min = numbers[i];
	}
	
	range = abs((max - min + 1)/NR_BUCKETS);
		
	/* Distribute numbers into buckets. */
	for (long i = 0; i < nnumbers; i++)
	{
		long j = (numbers[i] - min)/range;
		if (j >= NR_BUCKETS)
			j = NR_BUCKETS - 1;

		darray_append(buckets[j], numbers[i]);
	}
	
	/* Build indexes. */
	indexes[0] = 0;
	for (int i = 1; i < NR_BUCKETS; i++)
		indexes[i] = indexes[i - 1] + darray_size(buckets[i - 1]);
	
	profile_start();
	/* Sort Each bucket. */
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#elif defined(_SCHEDULE_SRR_)
	for (int i = 0; i < NR_BUCKETS; i++)
	{
		if (darray_size(buckets[i]) > UINT_MAX)
			error("bucket size overflow");
		_buckets_size[i] = darray_size(buckets[i]);
	}
	omp_set_workload(_buckets_size, NR_BUCKETS);
	#pragma omp parallel for schedule(runtime)
#endif
	for (int i = 0; i < NR_BUCKETS; i++)
	{
		if (darray_size(buckets[i]) == 0)
			continue;

		workload[omp_get_thread_num()] += darray_size(buckets[i]);
		
		is(buckets[i]);
	}
	
	profile_end();
	profile_dump();
	
	for (int i = 0; i < MAX_THREADS; i++)
	fprintf(stderr, "%ld\n", workload[i]);
	
	/* Rebuild array. */
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < NR_BUCKETS; i++)
	{
		long k = indexes[i];
		
		for (long j = 0; j < darray_size(buckets[i]); j++)
			numbers[k + j] = darray_get(buckets[i], j);
	}
	
	/* House keeping. */
	for (int i = 0; i < NR_BUCKETS; i++)
		darray_destroy(buckets[i]);
	free(buckets);
	free(indexes);
}
