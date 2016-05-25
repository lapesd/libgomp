/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MASTER_H_
#define MASTER_H_

	#include <arch.h>
	
	/*
	 * Matrix.
	 */
	struct matrix
	{
		int height;      /* Height.   */
		int width;       /* Width.    */
		float *elements; /* Elements. */
	};
	
	/*
	 * Opaque pointer to a matrix.
	 */
	typedef struct matrix * matrix_t;
	
	/*
	 * Caster to a matrix pointer.
	 */
	#define MATRIXP(m) \
		((matrix_t)(m))

	/*
	 * Returns the element [i][j] in a matrix.
	 */
	#define MATRIX(m, i, j)                                 \
		(MATRIXP(m)->elements[(i)*MATRIXP(m)->width + (j)]) \

	/*
	 * Creates a matrix.
	 */
	extern matrix_t matrix_create(int height, int width);
	
	/*
	 * Destroys a matrix.
	 */
	extern void matrix_destroy(matrix_t m);
	
	/*
	 * Performs LU factorization.
	 */
	extern int matrix_lu(matrix_t m, matrix_t l, matrix_t u);
	
	/*
	 * Fills up a matrix with random numbers.
	 */
	extern void matrix_random(matrix_t m);

	/*
	 * Close NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/*
	 * Joins slave processes.
	 */
	extern void join_slaves(void);
	
	/*
	 * Kills slave processes.
	 */
	extern void kill_slaves(void);
	
	/*
	 * Open NoC connectors.
	 */
	extern void open_noc_connectors(void);

	/*
	 * Spawns slave processes.
	 */
	extern void spawn_slaves(void);
	
	/*
	 * Waits for slaves to be ready.
	 */
	extern void sync_slaves(void);
	
	
	/* Channels. */
	extern int infd[NUM_CLUSTERS];  /* Input channels.  */
	extern int outfd[NUM_CLUSTERS]; /* Output channels. */

#endif /* MASTER_H_ */
