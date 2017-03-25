/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10

/*
 * For now, libgomp hopes that we will
 * fill these structures. A better
 * way to achieve the same think would
 * be to do something like:
 * 
 *   #pragma omp paralell for tasks(myarray, ntasks)
 */
unsigned *__tasks;
unsigned __ntasks;

int main(int argc, char **argv)
{
	int i;
	int cpt = 0;
	
	((void)argc);
	((void)argv);
	
	/* Initialize tasks. */
	__tasks = (unsigned *) malloc(N*sizeof(unsigned));
	__ntasks = N;
	for (i = 0; i < N; i++)
		__tasks[i] = i;
	
	#pragma omp parallel num_threads(2)
	{
		/*
		 * schedule(runtime) means the loop scheduler
		 * will be chosen at run time thanks to the
		 * OMP_SCHEDULE environement variable.
		 */
		#pragma omp for schedule(runtime)
		for (i=0; i<N; i++)
		{
			/* 
			 * parallel incr, unsafe!
			 * That's just a dummy example :-)
			 */
			cpt++;
		}
	}
	
	free(__tasks);

	return (0);
}
