/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/matrix_create() implementation.
 */

#include <assert.h>
#include <global.h>
#include <util.h>
#include "master.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK()  \
	assert(height > 0); \
	assert(width > 0);  \

/*
 * Creates a matrix.
 */
struct matrix *matrix_create(int height, int width)
{
	struct matrix *m;
	
	SANITY_CHECK();
	
	m = smalloc(sizeof(struct matrix));

	/* Initialize matrix. */
	m->height = height;
	m->width = width;
	m->elements = scalloc(height*width, sizeof(float));
	
	return (m);
}
