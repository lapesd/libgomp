/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/matrix_destroy.c - matrix_destroy() implementation.
 */

#include <assert.h>
#include <global.h>
#include <stdlib.h>
#include "master.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \

/*
 * Destroys a matrix.
 */
void matrix_destroy(struct matrix *m)
{
	SANITY_CHECK();
	
	free(m->elements);
	free(m);
}
