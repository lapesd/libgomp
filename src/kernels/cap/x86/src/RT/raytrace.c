/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include "image.h"
#include "sphere.h"
#include "vector.h"

#undef INFINITY
#undef PI

#define BIAS 0.001
#define PI 3.141592653589793
#define INFINITY FLT_MAX

static int max_depth;

static float max(float a, float b)
{
	return ((a > b) ? a : b);
}

static float min(float a, float b)
{
	return ((a < b) ? a : b);
}

static float mix(float a, float b, float mix)
{
	return (b * mix + a * (1.0 - mix));
}

/*
 * 
 */
struct vector raytrace(struct vector rayorig, struct vector raydir, sphere_t *spheres, int nspheres, int depth)
{
	int inside;
	int i, j;               /* Loop indexes. */
	float t0;
	float t1;
	sphere_t s;
	struct vector surface_color; /* Color of the surface.             */
	struct vector phit;          /* Point of intersection.            */
	struct vector nhit;          /* Normal at the intersection point. */
	struct vector lightdir;      /* Light direction. */
	struct vector refldir;
	struct vector refrdir;
	struct vector tmp1, tmp3;
	struct vector reflection;
	struct vector refraction;
	float tnear;
	float facingratio;
	float fresneleffect;
	float ior;
	float cosi;
	float k;
	float eta;
	int transmission;
	
	t0 = INFINITY;
	t1 = INFINITY;
	tnear = INFINITY;
	s = NULL;
	
	/*
	 * Find closest sphere in the scene 
	 * that the ray intercepts.
	 */
	for (i = 0; i < nspheres; i++)
	{
		/* This sphere is intercepted. */
		if (sphere_intersects(spheres[i], rayorig, raydir, &t0, &t1))
		{
			if (t0 < 0)
				t0 = t1;
			
			/* Closest sphere found. */
			if (t0 < tnear)
			{
				tnear = t0;
				s = spheres[i];
			}
		}
	}
	
	/*
	 * There is no intersection
	 * so return background color.
	 */
	if (s == NULL)
		return (VECTOR(2, 2, 2));
	
	surface_color = VECTOR(0, 0, 0);
	
	phit = vector_scalar(raydir, tnear);
	phit = vector_add(phit, rayorig);
	
	nhit = vector_sub(phit, sphere_center(s));
	nhit = vector_normalize(nhit);
	
	inside = 0;
	if (vector_dot(raydir, nhit) > 0)
	{
		nhit = vector_invert(nhit);
		inside = 1;
	}
										
	tmp3 = vector_scalar(nhit, BIAS);
	tmp3 = vector_add(tmp3, phit);

	if (((s->transparency > 0) || (s->reflection > 0)) && (depth < max_depth))
	{		
		facingratio = -vector_dot(raydir, nhit);
		fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
		
		tmp1 = vector_scalar(nhit, 2*vector_dot(raydir, nhit));
		refldir = vector_sub(raydir, tmp1);
		refldir = vector_normalize(refldir);
		
		reflection = raytrace(tmp3, refldir, spheres, nspheres, depth + 1);
		
		refraction = VECTOR(0, 0, 0);
		
		if (s->transparency > 0)
		{			
			ior = 1.1;
			eta = (inside) ? ior : 1/ior;
			cosi = -vector_dot(nhit, raydir);
			k = 1 - eta*eta*(1 - cosi*cosi);
			
			refrdir = vector_scalar(raydir, eta);
			tmp1 = vector_scalar(nhit, eta*cosi - sqrt(k));
			refrdir = vector_add(refrdir, tmp1);
			refrdir = vector_normalize(refrdir);
			
			tmp3 = vector_scalar(nhit, BIAS);
			tmp3 = vector_sub(phit, tmp3);
			
			refraction = raytrace(tmp3, refrdir, spheres, nspheres, depth + 1);
		}
		
		refraction =vector_scalar(refraction,(1-fresneleffect)*s->transparency);
		reflection = vector_scalar(reflection, fresneleffect);
		
		tmp1 = vector_add(reflection, refraction);
		surface_color = vector_cross(tmp1, s->surface_color);
	}
	
	else
	{	
		/*
		 * It is a diffuse object, so there
		 * is no need to raytrace any further.
		 */
		for (i = 0; i < nspheres; i++)
		{
			/* This is a light source. */
			if (spheres[i]->emission_color.x > 0)
			{
				transmission = 1;

				lightdir = vector_sub(spheres[i]->center, phit);
				lightdir = vector_normalize(lightdir);
				
				for (j = 0; j < nspheres; j++)
				{	
					if (i == j)
						continue;
					
					/* Shade this point. */
					if (sphere_intersects(spheres[j], tmp3, lightdir, NULL, NULL))
					{
						transmission = 0;
						break;
					}
				}
				
				if (transmission)
				{
					tmp1 = vector_scalar(s->surface_color, max(0, vector_dot(nhit, lightdir)));
					tmp1 = vector_cross(tmp1, spheres[i]->emission_color);
					
					surface_color = vector_add(surface_color, tmp1);
				}
			}
		}
	}

	return (vector_add(surface_color, s->emission_color));
}

image_t render(sphere_t *spheres, int nspheres, unsigned width, unsigned height, int depth)
{
	struct vector raydir;    /* Ray direction.  */
	unsigned x, y;
	float xx, yy;
	image_t img;
	float invwidth;     /* width^-1  */
	float invheight;    /* height^-1 */
	float angle;
	float fov;
	struct vector pixel;
	float aspectratio; /* Image's aspect ratio. */

	max_depth = depth;

	img = image_create(width, height);
	
	invwidth = 1.0 / width;
	invheight = 1.0 /height;
	fov = 30;

	aspectratio = width / ((float)height);
	angle = tan(PI*0.5*fov/180.0);

	for (y = 0; y < height; ++y)
	{
		#pragma omp parallel for \
			default(shared) private(x,xx,yy,raydir,pixel) schedule(dynamic)
		for (x = 0; x < width; x++)
		{
			xx = (2 * ((x + 0.5) * invwidth) - 1) * angle * aspectratio;
			yy = (1 - 2 * ((y + 0.5) * invheight)) * angle;
			
			raydir = vector_normalize(VECTOR(xx, yy, -1));
			
			pixel = raytrace(VECTOR(0, 0, 0), raydir, spheres, nspheres, 0);
			
			IMAGE(img, x, y).r = (unsigned char) (min(1.0, pixel.x)*255);
			IMAGE(img, x, y).g = (unsigned char) (min(1.0, pixel.y)*255);
			IMAGE(img, x, y).b = (unsigned char) (min(1.0, pixel.z)*255);
		}
	}
	
	return (img);
}
