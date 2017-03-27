
#include <assert.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <util.h>

extern void dump(const double *, int, const char *);

#include "lava.h"

#define DOT(A,B) ((A.x)*(B.x)+(A.y)*(B.y)+(A.z)*(B.z))	// STABLE

#define ALPHA 0.5
#define ALPHA2 (ALPHA*ALPHA)

#define NPARTICLES 100

#define NITERATIONS 1

//     14 APR 2011 Lukasz G. Szafaryn

extern void omp_set_workload(unsigned *, unsigned);

static void lava(box_str* box, int *npoints, int nboxes, FOUR_VECTOR* rv, double* qv, FOUR_VECTOR* fv, int nthreads)
{
	unsigned *_tasks;
	double loads[nthreads];
	double times[nthreads];

	/* Data allocation. */
	_tasks = smalloc(nboxes*sizeof(unsigned));

	memset(times, 0, nthreads*sizeof(double));
	memset(loads, 0, nthreads*sizeof(unsigned));

	/* Workload information. */
	for (int i = 0; i < nboxes; i++)
		_tasks[i] = npoints[i]*npoints[i]*(box[i].nn + 1);
	omp_set_workload(_tasks, nboxes);

	#pragma omp parallel num_threads(nthreads)
	{
		int tid;
		double start;
		double private_load, private_time;

		private_load = 0.0;
		private_time = 0.0;

		tid = omp_get_thread_num();
		
		#pragma omp	for schedule(runtime)
		for (int l = 0; l < nboxes; l++)
		{
			long first_i;
			FOUR_VECTOR *rA;
			FOUR_VECTOR *fA;
			
			private_load += npoints[l];

			start = omp_get_wtime();

			/* Get home box. */
			first_i = box[l].offset;

			/* Get home box paramemters. */
			rA = &rv[first_i];
			fA = &fv[first_i];

			/* Do for the # of (home + neighbor) boxes. */
			for (int k = 0; k < (box[l].nn + 1); k++) 
			{
				int pointer;
				long first_j; 
				FOUR_VECTOR *rB;
				double* qB;

				/* Get neighbor box. */
				pointer = (k == 0) ? l : box[l].nei[k - 1].number;
				first_j = box[pointer].offset; 

				/* Get neighbor box parameters. */
				rB = &rv[first_j];
				qB = &qv[first_j];

				for (int i = 0; i < npoints[l]*NPARTICLES; i++)
				{
					for (int j = 0; j < npoints[l]*NPARTICLES; j++)
					{
						double fs;
						double vij;
						double r2, u2;
						THREE_VECTOR d;
						double fxij, fyij, fzij;

						r2 = rA[i].v + rB[j].v - DOT(rA[i],rB[j]); 
						u2 = ALPHA2*r2;
						vij= exp(-u2);
						fs = 2.*vij;
						d.x = rA[i].x - rB[j].x; 
						d.y = rA[i].y - rB[j].y; 
						d.z = rA[i].z - rB[j].z; 
						fxij=fs*d.x;
						fyij=fs*d.y;
						fzij=fs*d.z;

						fA[i].v += qB[j]*vij;
						fA[i].x += qB[j]*fxij;
						fA[i].y += qB[j]*fyij;
						fA[i].z += qB[j]*fzij;
					}
				}
			}

		private_time += omp_get_wtime() - start;
	}

		loads[tid] = private_load;
		times[tid] = private_time;
	}


	dump(loads, nthreads, "load");
	dump(times, nthreads, "time");

	/* House keeping. */
	free(_tasks);
}

static void kernel_init( FOUR_VECTOR* rv, double* qv, FOUR_VECTOR* fv, int npoints)
{
	srand(0);

	/* Initialize input distances. */
	for (int i = 0; i < npoints; i++)
	{
		rv[i].v = (rand()%10 + 1)/10.0;
		rv[i].x = (rand()%10 + 1)/10.0;
		rv[i].y = (rand()%10 + 1)/10.0;
		rv[i].z = (rand()%10 + 1)/10.0;
	}

	/* Initialize input charges. */
	for (int i = 0; i < npoints; i++)
		qv[i] = (rand()%10 + 1)/10.0;

	/* Initialize output forces. */
	for (int i = 0; i < npoints; i++)
	{
		fv[i].v = 0;
		fv[i].x = 0;
		fv[i].y = 0;
		fv[i].z = 0;
	}
}

void kernel(int *tasks, int ntasks, int nthreads)
{
	box_str *boxes;
	FOUR_VECTOR *rv;
	double *qv;
	FOUR_VECTOR *fv;
	int nh;
	int nboxes1d;
	int nboxes;
	int npoints;
	int off;

	nboxes1d = cbrt(ntasks);
	nboxes = nboxes1d*nboxes1d*nboxes1d;

	printf("Configuration used: cores = %d, nboxes = %d, ntasks = %d\n", nthreads, nboxes, ntasks);
	assert(ntasks == nboxes);

	((void) tasks);
	((void) ntasks);

	/* Compute the number of points. */
	npoints = 0;
	for (int i = 0; i < ntasks; i++)
		npoints += tasks[i]*NPARTICLES;

	/* Data allocation.  */
	boxes = (box_str*)malloc(nboxes*sizeof(box_str));
	rv = (FOUR_VECTOR*)malloc(npoints*sizeof(FOUR_VECTOR));
	qv = (double*)malloc(npoints*sizeof(double));
	fv = (FOUR_VECTOR*)malloc(npoints*sizeof(FOUR_VECTOR));

	nh = 0;

	off = 0;
	for (int i = 0; i < nboxes1d; i++)
	{
		for (int j = 0; j < nboxes1d; j++)
		{
			for (int k = 0; k < nboxes1d; k++)
			{
				boxes[nh].x = k;
				boxes[nh].y = j;
				boxes[nh].z = i;
				boxes[nh].number = nh;
				boxes[nh].offset = off;

				/* Initialize neighbor boxes. */
				boxes[nh].nn = 0;
				for (int l = -1; l < 2; l++)
				{
					for (int m = -1; m < 2; m++)
					{
						for (int n = -1; n < 2; n++)
						{
							/* Skip home box. */
							if ((l == 0) && (m == 0) && (n == 0))
								continue;

							/* Invalid neighbor box. */
							if (((i + l) < 0) || ((i + l) >= nboxes1d) ||
							    ((j + m) < 0) || ((j + m) >= nboxes1d) ||
							    ((k + n) < 0) || ((k + n) >= nboxes1d))
								continue;

							boxes[nh].nei[boxes[nh].nn].x = (k+n);
							boxes[nh].nei[boxes[nh].nn].y = (j+m);
							boxes[nh].nei[boxes[nh].nn].z = (i+l);
							boxes[nh].nei[boxes[nh].nn].number = (boxes[nh].nei[boxes[nh].nn].z * nboxes1d * nboxes1d) + 
								                                     (boxes[nh].nei[boxes[nh].nn].y * nboxes1d) + 
																	  boxes[nh].nei[boxes[nh].nn].x;
							
							boxes[nh].nei[boxes[nh].nn].offset = boxes[nh].nei[boxes[nh].nn].number * tasks[nh]*NPARTICLES;

							boxes[nh].nn++;
						}
					}
				}

				off += tasks[nh]*NPARTICLES;
				nh++;
			}
		}
	}

	kernel_init(rv, qv, fv, npoints);

	for (int i = 0; i < NITERATIONS; i++)
		lava(boxes, tasks, nboxes, rv, qv, fv, nthreads);

	/* House keeping. */
	free(rv);
	free(qv);
	free(fv);
	free(boxes);
}
