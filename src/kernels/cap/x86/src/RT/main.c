/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <timer.h>
#include "image.h"
#include "sphere.h"
#include "vector.h"

/*
 * Problem.
 */
struct problem
{
	int height;     /* Image height. */
	int width;      /* Image width.  */
	unsigned depth; /* Ray depth. */
};

/* Problem sizes. */
static struct problem tiny     =  { 1280,  720, 10 };
static struct problem small    =  { 1600,  900, 30 };
static struct problem standard =  { 1920, 1080, 50 };
static struct problem large    =  { 2048, 1152, 70 };
static struct problem huge     =  { 2560, 1440, 90 };

/* Benchmark parameters. */      
int verbose = 0;                  /* Be verbose?        */    
int nthreads = 1;                 /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */     

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: rt [options]\n");
	printf("Brief: Ray Tracing Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nthreads <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - tiny\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_NTHREADS 1 /* Set number of threads. */
	#define SET_CLASS    2 /* Set problem class.     */
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else if (!strcmp(argv[i], "huge"))
						p = &huge;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_NTHREADS :
					nthreads = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nthreads"))
			state = SET_NTHREADS;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nthreads < 1)
		usage();
}

extern image_t render(sphere_t *spheres, int nspheres, unsigned width, unsigned height, int depth);


/*
 * RT kernel.
 */
int main(int argc, char **argv)
{
	/* Number of spheres. */
	#define NR_SPHERES 6
	
	int i;                        /* Loop index. */
	image_t img;                  /* Image.      */
	sphere_t spheres[NR_SPHERES]; /* Spheres.    */
	uint64_t end;                 /* End time.   */
	uint64_t start;               /* Start time. */
	
#ifdef _XEON_PHI_
	double power;
#endif
	
	readargs(argc, argv);
	
	timer_init();
	omp_set_num_threads(nthreads);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	/* Ground sphere. */
	spheres[0] = sphere_create(
					VECTOR(0, -10004, -20), /* Center         */
					10000,                  /* Radius         */
					VECTOR(0.2, 0.2, 0.2),  /* Surface Color  */
					0,                      /* Reflection     */
					0,                      /* Transparency   */
					VECTOR(0, 0, 0));       /* Emission Color */
	
	/* Red sphere. */
	spheres[1] = sphere_create(
					VECTOR(0, 0, -20),        /* Center         */
					4,                        /* Radius         */
					VECTOR(1.00, 0.32, 0.36), /* Surface Color  */
					1,                        /* Reflection     */
					0.5,                      /* Transparency   */
					VECTOR(0, 0, 0));         /* Emission Color */
	
	/* Yellow sphere. */
	spheres[2] = sphere_create(
					VECTOR(5, -1, -15),
					2,
					VECTOR(0.90, 0.76, 0.46),
					1,
					0.0,
					VECTOR(0, 0, 0));
	
	/* Blue sphere. */
	spheres[3] = sphere_create(
					VECTOR(5, 0, -25),
					3,
					VECTOR(0.65, 0.77, 0.97),
					1,
					0.0,
					VECTOR(0, 0, 0));
	
	/* Gray sphere. */
	spheres[4] = sphere_create(
					VECTOR(-5.5, 0, -15),
					3,
					VECTOR(0.90, 0.90, 0.90),
					1,
					0.0,
					VECTOR(0, 0, 0));
	
	/* Light source. */
	spheres[5] = sphere_create(
					VECTOR(0, 30, -30),
					3,
					VECTOR(0, 0, 0),
					0,
					0,
					VECTOR(3, 3, 3));
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
#ifdef _XEON_PHI_
	power_init();
#endif
	
	/* Ray tracing. */
	if (verbose)
		printf("rendering scene...\n");
	start = timer_get();
	img = render(spheres, NR_SPHERES, p->height, p->width, p->depth);
	end = timer_get();
	
#ifdef _XEON_PHI_
	power = power_end();
#endif

	if (verbose)
		image_export("out.ppm", img, IMAGE_PPM);
	
	printf("timing statistics:\n");
	printf("  total time:    %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	printf("  average power: %f\n", power*0.000001);
#endif
	
	/* Hous keeping. */
	for (i = 0; i < NR_SPHERES; i++)
		sphere_destroy(spheres[i]);
	image_destroy(img);
	
	return (EXIT_SUCCESS);
}
