/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef VECTOR_H_
#define VECTOR_H_

	/*
	 * 3D Vector.
	 */
	struct vector
	{
		float x; /* x coordinate. */
		float y; /* y coordinate. */
		float z; /* z coordinate. */
	};
	
	extern struct vector VECTOR(float x, float y, float z);
	
	/*
	 * Adds v2 to v1.
	 */
	extern struct vector vector_add(struct vector v1, struct vector v2);
	
	/*
	 * Cross product.
	 */
	extern struct vector vector_cross(struct vector v1, struct vector v2);
	
	/*
	 * Dot product.
	 */
	extern float vector_dot(struct vector v1, struct vector v2);
	
	/*
	 * Multiplies a 3D vector by -1.
	 */
	extern struct vector vector_invert(struct vector v);
	
	/*
	 * Normalizes a 3D vector.
	 */
	extern struct vector vector_normalize(struct vector v);
	
	/*
	 * Scalar product.
	 */
	extern struct vector vector_scalar(struct vector v, float a);
	
	/*
	 * Subtracts v2 from v1.
	 */
	extern struct vector vector_sub(struct vector v1, struct vector v2);

#endif /* VECTOR_H_ */
