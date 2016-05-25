/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * lu.c - Lower Upper Benchmark Kerkernl.
 */

#include <assert.h>
#include <global.h>
#include <math.h>
#include <omp.h>
#include <util.h>
#include "lu.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \
	assert(l != NULL); \
	assert(u != NULL); \

/*
 * Swaps two rows of a matrix.
 */
static void _swap_rows(struct matrix *m, int i1, int i2)
{
	int j;     /* Loop index.      */
	float tmp; /* Temporary value. */
	
	/* Swap columns. */
	#pragma omp parallel for private(j, tmp) default(shared)
	for (j = 0; j < m->width; j++)
	{
		tmp = MATRIX(m, i1, j);
		MATRIX(m, i1, j) = MATRIX(m, i2, j);
		MATRIX(m, i2, j) = tmp;
	}
}

/*
 * Swaps two columns in a matrix.
 */
static void _swap_columns(struct matrix *m, int j1, int j2)
{
	int i;     /* Loop index.      */
	float tmp; /* Temporary value. */

	/* Swap columns. */
	#pragma omp parallel for private(i, tmp) default(shared)
	for (i = 0; i < m->height; i++)
	{
		tmp = MATRIX(m, i, j1);
		MATRIX(m, i, j1) = MATRIX(m, i, j2);
		MATRIX(m, i, j2) = tmp;
	}
}

/*
 * Finds the pivot element.
 */
static float _find_pivot(struct matrix *m, int i0, int j0)
{
	int i, j;         /* Loop indexes.          */
	int ipvt, jpvt;   /* Pivot indexes.         */
	int pipvt, pjpvt; /* Private pivot indexes. */
	
	ipvt = i0;
	jpvt = j0;
	
	#pragma omp parallel private(i, j, pipvt, pjpvt) shared(m, i0, j0)
	{
		pipvt = i0;
		pjpvt = j0;
		
		/* Find pivot element. */
		#pragma omp for
		for (i = i0; i < m->height; i++)
		{
			for (j = j0; j < m->width; j++)
			{
				/* Found. */
				if (fabs(MATRIX(m, i, j)) < fabs(MATRIX(m,pipvt,pjpvt)))
				{
					pipvt = i;
					pjpvt = j;
				}
			}
		}
		
		/* Reduct. */
		#pragma omp critical
		{
			if (fabs(MATRIX(m, pipvt, pjpvt) > fabs(MATRIX(m, ipvt, jpvt))))
			{
				ipvt = pipvt;
				jpvt = pjpvt;
			}
		}
	}
	
	_swap_rows(m, i0, ipvt);
	_swap_columns(m, j0, jpvt);
	
	return (MATRIX(m, ipvt, jpvt));
}

/*
 * Applies the row reduction algorithm in a matrix.
 */
static void _row_reduction(struct matrix *m, int i0, float pivot)
{
	int j0;      /* Starting column. */
	int i, j;    /* Loop indexes.    */
	float mult;  /* Row multiplier.  */
	
	j0 = i0;
	
	/* Apply row redution in some lines. */
	#pragma omp parallel for private(i, j, mult) default(shared)
	for (i = i0 + 1; i < m->height; i++)
	{
		mult = MATRIX(m, i, j0)/pivot;
	
		/* Store multiplier. */
		MATRIX(m, i, j0) = mult;
	
		/* Iterate over columns. */
		for (j = j0 + 1; j < m->width; j++)
			MATRIX(m, i, j) = MATRIX(m, i, j) - mult*MATRIX(m, i0, j);
	}
}

/*
 * Performs LU factorization in a matrix.
 */
int lower_upper(struct matrix *m, struct matrix *l, struct matrix *u)
{
	int i, j;    /* Loop indexes. */
	float pivot; /* Pivot.        */
	
	/* Apply elimination on all rows. */
	for (i = 0; i < m->height - 1; i++)
	{
		pivot = _find_pivot(m, i, i);	
	
		/* Impossible to solve. */
		if (pivot == 0.0)
		{
			warning("cannot factorize matrix");
			return (-1);
		}
		
		_row_reduction(m, i, pivot);
	}
	
	/* Build upper and lower matrixes.  */
	#pragma omp parallel for private(i, j) default(shared)
	for (i = 0; i < m->height; i++)
	{
		for (j = 0; j < m->width; j++)
		{
			if (i > j)
			{
				MATRIX(l, i, j) = 0.0;
				MATRIX(u, i, j) = MATRIX(m, i, j);
			}
			
			else if (i < j)
			{	
				MATRIX(l, i, j) = MATRIX(m, i, j);
				MATRIX(u, i, j) = 0.0;
			}
			
			else
			{
				MATRIX(l, i, j) = 1.0;
				MATRIX(u, i, j) = 1.0;
			}
		}
	}
	
	return (0);
}
