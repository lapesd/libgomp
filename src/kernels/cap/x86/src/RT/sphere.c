/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <math.h>
#include <util.h>
#include "sphere.h"
#include "vector.h"

/*
 * Returns the center of a sphere.
 */
struct vector sphere_center(struct sphere *s)
{
	/* Sanity check. */
	assert(s != NULL);
	
	return (s->center);
}

/*
 * Creates a sphere.
 */
struct sphere *sphere_create
(struct vector c, float r, struct vector sc, float rf, float t, struct vector ec)
{
	struct sphere *s;
	
	/* Sanity check. */
	assert(r > 0);
	assert(t >= 0);
	assert(t >= 0);
	
	s = smalloc(sizeof(struct sphere));
	
	/* Initialize sphere. */
	s->radius = r;
	s->radius2 = r*r;
	s->transparency = t;
	s->reflection = rf;
	s->surface_color = sc;
	s->emission_color = ec;
	s->center = c;
	
	return (s);
}

/*
 * Destroys a sphere.
 */
void sphere_destroy(struct sphere *s)
{
	/* Sanity check. */
	assert(s != NULL);
	
	free(s);
}

/*
 * Asserts if a ray intercepts a sphere.
 */
int sphere_intersects
(struct sphere *s, struct vector rayorig, struct vector raydir, float *t0, float *t1)
{
	float d2;
	float tca;
	float thc;
	struct vector l;
	
	/* Sanity check. */
	assert(s != NULL);
	
	l = vector_sub(s->center, rayorig);
	
	tca = vector_dot(l, raydir);
	
	if (tca < 0)
		return (0);
	
	d2 = vector_dot(l, l) - tca*tca;
	
	if (d2 > s->radius2)
		return (0);
	
	thc = sqrt(s->radius2 -d2);
	
	if ((t0 != NULL) && (t1 != NULL))
	{
		*t0 = tca - thc;
		*t1 = tca + thc;
	}
	
	return (1);
}
