/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.h -  Private slave library.
 */

#ifndef _SLAVE_H_
#define _SLAVE_H_

	#include <stdlib.h>

	/*
	 * Synchronizes with master process.
	 */
	extern void sync_master(void);
	
	/*
	 * Opens NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Closes NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/* Inter process communication. */
	extern int rank;  /* Process rank.   */
	extern int infd;  /* Input channel.  */
	extern int outfd; /* Output channel. */
	
	/*
	 * Calculates the distance between two points.
	 */
	extern float vector_distance(float *a, float *b);
	
	/*
	 * Adds two vectors.
	 */
	extern float *vector_add(float *v1, const float *v2);
	
	/*
	 * Multiplies a vector by a scalar.
	 */
	extern float *vector_mult(float *v, float scalar);
	
	/*
	 * Assigns a vector to another.
	 */
	extern float *vector_assign(float *v1, const float *v2);
	
	/*
	 * Tests if two vectors are equal.
	 */
	extern int vector_equal(const float *v1, const float *v2);
	
#endif /* _SLAVE_H_ */
