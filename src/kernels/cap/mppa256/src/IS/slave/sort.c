/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/sort.c - sort() implementation.
 */

#include <stdio.h>
#include <arch.h>

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

#define M 10

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
void _sort(int *a, int n)
{
	quicksort(a, 0, n - 1);
	insertion(a, 0, n - 1);
}

/*
 * Merges two arrays.
 */
void merge(int *a, int *b, int size)
{
	int tmp; /* Temporary value. */
	int i, j;/* Loop indexes.    */
	
	
	/* Merge. */
	i = 0; j = 0;
	while ((i < size) && (j < size))
	{
		if (b[j] < a[i])
		{
			exch(b[j], a[i], tmp)
			j++;
		}
		else
		{
			i++;
		}
	}
}

/*
 * Mergesort algorithm.
 */
void sort2power(int *array, int size, int chunksize)
{
	int i; /* Loop index.         */
	int N; /* Working array size. */

	/* Sort. */
	#pragma omp parallel for private(i) default(shared)
	for (i = 0; i < size; i += chunksize)
		_sort(&array[i], (i + chunksize < size) ? chunksize : size - i);

	/* Merge. */
	for (N = chunksize; N < size; N = N + N)
	{
		#pragma omp parallel for private(i) default(shared)
		for (i = 0; i < size; i += N+N)
		{
			/* TODO: allow non-multiple of 2. */
			merge(&array[i], &array[i + N], N);
		}
	}
}
