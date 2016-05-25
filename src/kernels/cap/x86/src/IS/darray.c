/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * darray.c - Dynamic array library implementation.
 */

#include <util.h>
#include "is.h"

/*
 * Creates a dynamic array.
 */
struct darray *darray_create(int size)
{
	struct darray *da;
	
	da = smalloc(sizeof(struct darray));
	
	/* Initialize array. */
	da->size = 0;
	da->maxsize = size;
	da->elements = scalloc(size, sizeof(int));
	
	return (da);
}
	
/*
 * Destroys a dynamic array.
 */
void darray_destroy(struct darray *da)
{
	free(da->elements);
	free(da);
}

/*
 * Expands a dynamic array.
 */
static void darray_expand(struct darray *da)
{
	da->maxsize <<= 1;
	da->elements = srealloc(da->elements, sizeof(int)*da->maxsize);
}

/*
 * Appends a value to a dynamic array.
 */
void darray_append(struct darray *da, int value)
{
	/* Expand array. */
	if (da->size == da->maxsize)
		darray_expand(da);
		
	da->elements[da->size++] = value;
}

/*
 * Gets a value from a dynamic array.
 */
int darray_get(struct darray *da, int idx)
{
	if (idx >= da->size || idx < 0)
		error("darray_get() out of bounds");
  
  return (da->elements[idx]);
}

/*
 * Sets a value in a dynamic array.
 */
void darray_set(struct darray *da, int idx, int value)
{
	/* Expand array. */
	while (idx >= da->size)
		darray_append(da, 0);

	da->elements[idx] = value;
}

