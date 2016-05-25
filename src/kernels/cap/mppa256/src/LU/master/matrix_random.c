/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/matrix_random.c - matrix_random() implementation.
 */

#include <assert.h>
#include <util.h>
#include "master.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \

/*
 * Fills up a matrix with random numbers.
 */
void matrix_random(struct matrix *m)
{
	int i, j;
	
	SANITY_CHECK();
	
	/* Fill matrix. */
	for (i = 0; i < m->height; i++)
	{
		for (j = 0; j < m->width; j++)
			MATRIX(m, i, j) = randnum();
	}
}
