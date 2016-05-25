/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef SPHERE_H_
#define SPHERE_H_

	#include "vector.h"

	/*
	 * Sphere.
	 */
	struct sphere
	{
		float radius;            /* Radius.         */
		float radius2;           /* Radius^2.       */
		float transparency;      /* Transparency.   */
		float reflection;        /* Reflection.     */
		struct vector surface_color;  /* Surface color.  */
		struct vector emission_color; /* Emission color. */
		struct vector center;         /* Center.         */
	};
	
	/*
	 * Opaque pointer to a sphere.
	 */
	typedef struct sphere * sphere_t;
	
	/*
	 * Returns the center of a sphere.
	 */
	extern struct vector sphere_center(sphere_t s);
	
	/*
	 * Creates a sphere.
	 */
	extern sphere_t sphere_create
	(struct vector c, float r, struct vector sc, float rf, float t, struct vector ec);
	
	/*
	 * Destroys a sphere.
	 */
	extern void sphere_destroy(sphere_t s);
	
	/*
	 * Asserts if a ray intercepts a sphere.
	 */
	extern int sphere_intersects
	(sphere_t s, struct vector rayorig, struct vector raydir, float *t0, float *t1);

#endif /* SPHERE_H_ */
