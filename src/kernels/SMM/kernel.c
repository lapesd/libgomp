/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <assert.h>
#include <omp.h>

#include <util.h>
#include <profile.h>

/**
 * Kernel arguments.
 */
static struct
{
	int n;           /*< Matrix dimension. */
	const double *a; /*< Matrix A.         */
	const double *b; /*< Matrix B.         */
	double *c;       /*< Resulting matrix. */
} args = {0, NULL, NULL, NULL};

/**
 * @brief Short-hand for accessing element [i,j] of a matrix.
 *
 * @param m Target matrix.
 * @param i Row index.
 * @param j Column index.
 */
#define M(m, i, j) \
 	m[(i)*args.n + (j)]

/**
 * @brief Initialize the kernel.
 *
 * @param a Matrix A.
 * @param b Matrix B.
 * @param n Matrix dimension.
 */
void smm_init(const double *a, const double *b, int n)
{
	/* Sanity check. */
	assert(a != NULL);
	assert(b != NULL);
	assert(n > 0);

	/* Initialize kernel. */
	args.n = n;
	args.a = a;
	args.b = b;
	args.c = smalloc(n*n*sizeof(double));
}

/**
 * @brief Cleans up kernel stuff.
 */
void smm_end(void)
{
	/* Sanity check. */
	assert(args.c != NULL);

	free(args.c);
}

/**
 * @brief Runs the sparse matrix multiplication kernel.
 *
 *@param it Iteration number.
 */
void smm_run(int it)
{
	/* Sanity check. */
	assert(args.c != NULL);

	if (it)
		profile_start();

	/* Perform matrix multiplication. */
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#endif
	for (int i = 0; i < args.n; i++)
	{
		for (int j = 0; j < args.n; j++)
		{
			M(args.c, i, j) = 0.0;

			for (int k = 0; k < args.n; k++)
			{
				/* Skip this row. */
				if (M(args.a, i, k) == 0.0)
					continue;

				M(args.c, i, j) += M(args.a, i, k)*M(args.b, k, i);
			}
		}
	}
	
	if (it)
	{
		profile_end();
		profile_dump();
	}
}
