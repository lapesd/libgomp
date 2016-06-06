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
#define NR_BUCKETS 8

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
static int partition(int *a, int l, int r)
{
	int v;    /* Partitioning element. */
	int t;    /* Temporary element.    */
	int i, j; /* Loop index.           */
	
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
static void quicksort(int *a, int l, int r)
{
	int i; /* Pivot.             */
	int t; /* Temporary element. */
	
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
static void insertion(int *a, int l, int r)
{
	int t;    /* Temporary value. */
	int v;    /* Working element. */
	int i, j; /* Loop indexes.    */
	
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
void integer_sort(int *numbers, int nnumbers)
{
	int max, _max;           /* Max number in numbers. */
	int range;               /* Bucket range.          */
	int i, j, k;             /* Loop indexes.          */
	int *indexes;            /* Index for buckets.     */
	struct darray **buckets; /* Buckets.               */

	indexes = smalloc(NR_BUCKETS*sizeof(int));

	/* Create buckets. */
	buckets = smalloc(NR_BUCKETS*sizeof(struct darray *));
	for (i = 0; i < NR_BUCKETS; i++)
		buckets[i] = darray_create(nnumbers/NR_BUCKETS);
	
	max = INT_MIN;
	
	#pragma omp parallel private(i, j, k, _max)
	{	
		_max = INT_MIN;

		/* Find max number in the array. */
		#pragma omp for schedule(static)
		for (i = 0; i < nnumbers; i++)
		{
			/* Found. */
			if (numbers[i] > _max)
				_max = numbers[i];
		}

		#pragma omp critical
		{
			if (_max > max) {
				max = _max;
			}
		}
		
		#pragma omp master
		range = max/NR_BUCKETS;
		#pragma omp barrier
		
		/* Distribute numbers into buckets. */
		#pragma omp master
		for (i = 0; i < nnumbers; i++)
		{
			j = numbers[i]/range;
			if (j >= NR_BUCKETS)
				j = NR_BUCKETS - 1;

			darray_append(buckets[j], numbers[i]);
		}
		
		/* Sort Each bucket. */
		#if defined(_SCHEDULE_STATIC_)
		#pragma omp parallel for schedule(static)
	#elif defined(_SCHEDULE_GUIDED_)
		#pragma omp parallel for schedule(guided)
	#elif defined(_SCHEDULE_DYNAMIC_)
		#pragma omp parallel for schedule(dynamic)
	#elif defined(_SCHEDULE_SRR_)
		int _buckets_size[NR_BUCKETS];
		memcpy(_buckets_size, &_buckets_size[1], NR_BUCKETS*sizeof(int));
		omp_set_workload((unsigned *)&_buckets_size[1], NR_BUCKETS);
		#pragma omp parallel for schedule(runtime)
	#endif
		for (i = 0; i < NR_BUCKETS; i++)
		{
			if (darray_size(buckets[i]) > 0)
				is(buckets[i]);
		}
		
		#pragma omp master
		{
			/* Build indexes. */
			indexes[0] = 0;
			for (i = 1; i < NR_BUCKETS; i++)
				indexes[i] = indexes[i - 1] + darray_size(buckets[i]);
		
			/* Rebuild array. */
			for (i = 0; i < NR_BUCKETS; i++)
			{
				k = indexes[i];
					
				for (j = 0; j < darray_size(buckets[i]); j++)
					numbers[k + j] = darray_get(buckets[i], j);
			}
		}
	}
	
	/* House keeping. */
	for (i = 0; i < NR_BUCKETS; i++)
		darray_destroy(buckets[i]);
	free(buckets);
	free(indexes);
}
