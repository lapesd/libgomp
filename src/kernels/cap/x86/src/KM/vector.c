/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * vector.c - Vector Library implementation.
 */

#include <math.h>
#include <string.h>
#include <util.h>
#include "km.h"

/*
 * Creates a vector.
 */
struct vector *vector_create(int n)
{
	struct vector *v;
	
	v = smalloc(sizeof(struct vector));
	
	/* Initilize vector. */
	v->size = n;
	v->elements = calloc(n, sizeof(float));

	return (v);
}

/*
 * Adds two vectors.
 */
struct vector *vector_add(struct vector *v1, const struct vector *v2)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	n = vector_size(v1);
	
	/* Add vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v1, i) += VECTOR(v2, i);
	
	return (v1);
}

/*
 * Assigns a vector to another.
 */
struct vector *vector_assign(struct vector *v1, const struct vector *v2)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	n = vector_size(v1);
	
	/* Add vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v1, i) = VECTOR(v2, i);
	
	return (v1);
}

/*
 * Destroys a vector.
 */
void vector_destroy(struct vector *v)
{
	free(v->elements);
	free(v);
}

/*
 * Computes the euclidean distance between two points.
 */
float vector_distance(const struct vector *a, const struct vector *b)
{
	int i;          /* Loop index. */
	float distance; /* Distance.   */

	distance = 0;

	/* Computes the euclidean distance. */
	for (i = 0; i < a->size; i++)
		distance +=  pow(VECTOR(a, i) - VECTOR(b, i), 2);
	distance = sqrt(distance);
	
	return (distance);
}

/*
 * Tests if two vectors are equal.
 */
int vector_equal(const struct vector *a, const struct vector *b)
{
	int i;
	
	/* Test all elements. */
	for (i = 0; i < a->size; i++)
	{
		if (VECTOR(a, i) != VECTOR(b, i))
			return (0);
	}
	
	return (1);
}

/*
 * Multiplies a vector by a scalar.
 */
struct vector *vector_mult(struct vector *v, float scalar)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	n = vector_size(v);
	
	/* Add vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v, i) *= scalar;
	
	return (v);
}

/*
 * Fills up vector with random numbers.
 */
struct vector *vector_random(struct vector *v)
{
	int i;
	
	/* Fill vector. */
	for (i = 0; i < vector_size(v); i++)
		VECTOR(v, i) = randnum() & 0xffff;

	return (v);
}

/*
 * Subtracts two vectors.
 */
struct vector *vector_sub(struct vector *v1,const struct vector *v2)
{
	int i; /* Loop index.  */
	int n; /* Vector size. */
	
	/* Invalid argument. */
	if (vector_size(v1) != vector_size(v2))
		return (NULL);
	
	n = vector_size(v1);
	
	/* Subtract vectors. */
	for (i = 0; i < n; i++)
		VECTOR(v1, i) -= VECTOR(v2, i);
	
	return (v1);
}

/*
 * Clears a vector.
 */
void vector_clear(struct vector *v)
{
	memset(v->elements, 0, sizeof(float)*vector_size(v));	
}

