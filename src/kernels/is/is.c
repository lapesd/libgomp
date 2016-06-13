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

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

/**
 * @brief Number of buckets.
 * 
 * @details Number of buckets. Adjust this to improve cache locality.
 */
#define NR_BUCKETS 32

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

#define M 64

/*
 * Quicksort partition.
 */
static long partition(int *a, long l, long r)
{
	int v;     /* Partitioning element. */
	int t;     /* Temporary element.    */
	long i, j; /* Loop index.           */
	
	i = l - 1;
	j = r;
	v = a[r];
	
	while (1)
	{
		while (a[++i] < v)
			/* NOOP.*/ ;
		
		while (a[--j] > v)
		{
			if (j == l)
				break;
		}
		
		if (i >= j)
			break;
		
		exch(a[i], a[j], t);
	}
	
	exch(a[i], a[r], t);
	
	return (i);
}

/*
 * Quicksort.
 */
static void quicksort(int *a, long l, long r)
{
	long i; /* Pivot.             */
	int t;  /* Temporary element. */
	
	/* Fine grain stop. */
	if ((r - l) <= M)
		return;
	
	/* Avoid anomalous partition. */
	exch(a[(l + r) >> 1], a[r - 1], t);
	
	/* Median of three. */
	compexgh(a[l], a[r - 1], t);
	compexgh(a[l], a[r], t);
	compexgh(a[r - 1], a[r], t);
	
	/* Sort. */
	i = partition(a, l + 1 , r - 1);
	quicksort(a, l, i - 1);
	quicksort(a, i + 1, r);
}

/*
 * Insertion sort.
 */
static void insertion(int *a, long l, long r)
{
	int t;     /* Temporary value. */
	int v;     /* Working element. */
	long i, j; /* Loop indexes.    */
	
	for (i = r; i > l; i--)
		compexgh(a[i - 1], a[i], t);
	
	for (i = l + 2; i <= r; i++)
	{
		j = i;
		v = a[i];
		
		while (v < a[j - 1])
		{
			a[j] = a[j - 1];
			j--;
		}
		
		a[j] = v;
	}
}

/*
 * Sorts an array of numbers.
 */
void is(struct darray *da)
{
	quicksort(da->elements, 0, da->size - 1);
	insertion(da->elements, 0, da->size - 1);
}

/**
 * @brief integer sort kernel.
 */
void integer_sort(long *numbers, long nnumbers)
{
	int min, max;            /* Max and min numbers.   */
	int range;               /* Bucket range.          */
	long *indexes;           /* Index for buckets.     */
	struct darray **buckets; /* Buckets.               */

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
	
	profile_start();
		
	/* Sort Each bucket. */
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#elif defined(_SCHEDULE_SRR_)
	long _buckets_size1[NR_BUCKETS];
	memcpy(_buckets_size1, &_buckets_size1[1], NR_BUCKETS*sizeof(long));
	omp_set_workload((unsigned *)&_buckets_size1[1], NR_BUCKETS);
	#pragma omp parallel for schedule(runtime)
#endif
	for (int i = 0; i < NR_BUCKETS; i++)
	{
		if (darray_size(buckets[i]) == 0)
			continue;
		
		is(buckets[i]);
	}
	
	profile_end();
	profile_dump();
		
	/* Build indexes. */
	indexes[0] = 0;
	for (int i = 1; i < NR_BUCKETS; i++)
		indexes[i] = indexes[i - 1] + darray_size(buckets[i]);
	
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
