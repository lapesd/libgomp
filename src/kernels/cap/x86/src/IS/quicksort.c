/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/sort.c - sort() implementation.
 */

#include "is.h"

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
void sort(struct darray *da)
{
	quicksort(da->elements, 0, da->size - 1);
	insertion(da->elements, 0, da->size - 1);
}
