/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * master.h -  Private master library.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

	#include <arch.h>
	#include <stdlib.h>
	
	/*
	 * Vector.
	 */
	struct vector
	{
		int size;        /* Size.     */
		float *elements; /* Elements. */
	};
	
	/*
	 * Opaque pointer to a vector.
	 */
	typedef struct vector * vector_t;
	
	/*
	 * Opaque pointer to a constant vector.
	 */
	typedef const struct vector * const_vector_t;
	
	/*
	 * Returns the size of a vector.
	 */
	#define vector_size(v) \
		(((vector_t) (v))->size)
	
	/*
	 * Clears a vector.
	 */
	extern void vector_clear(struct vector *v);
	
	/*
	 * Creates a vector.
	 */
	extern vector_t vector_create(int n);
	
	/*
	 * Destroys a vector.
	 */
	extern void vector_destroy(vector_t v);
	
	/*
	 * Computes the euclidean distance between two points.
	 */
	extern float vector_distance(const_vector_t a, const_vector_t b);
	
	/*
	 * Tests if two vectors are equal.
	 */
	extern int vector_equal(const_vector_t a, const_vector_t b);
	
	/*
	 * Assigns a vector to another.
	 */
	extern vector_t vector_assign(vector_t v1, const_vector_t v2);
	
	/*
	 * Subtracts two vectors.
	 */
	extern vector_t vector_sub(vector_t v1, const_vector_t v2);
	
	/*
	 * Adds two vectors.
	 */
	extern vector_t vector_add(vector_t v1, const_vector_t v2);
	
	/*
	 * Multiplies a vector by a scalar.
	 */
	extern vector_t vector_mult(vector_t v, float scalar);
	
	/*
	 * Fills up vector with random numbers.
	 */
	extern vector_t vector_random(vector_t v);

	/*
	 * Returns the element [i] in a vector.
	 */
	#define VECTOR(v, i) \
		(((vector_t)(v))->elements[(i)])
	
	/*
	 * Spwans slave processes.
	 */
	extern void spawn_slaves(void);
	
	/*
	 * Joins slave processes.
	 */
	extern void join_slaves(void);
	
	/*
	 * Waits for slaves to be ready.
	 */
	extern void sync_slaves(void);
	
	/*
	 * Open NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Close NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/* Interprocess communication. */
	extern int infd[NUM_CLUSTERS];  /* Input channels.  */
	extern int outfd[NUM_CLUSTERS]; /* Output channels. */

#endif /* _MASTER_H_ */
