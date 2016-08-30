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

#include <stdio.h>

#include <omp.h>

#include <util.h>

/**
 * @brief Time.
 */
static double totaltime = 0.0;
static double residual = 0.0;

/**
 * @brief Start profiling.
 */
void profile_start(void)
{
	double start = 0.0;
	double end = 0.0;
	
	start = omp_get_wtime();
	end = omp_get_wtime();
	
	residual = end - start;
	
	totaltime = omp_get_wtime();
}

/**
 * @brief End profiling.
 */
void profile_end(void)
{
	totaltime = omp_get_wtime() - totaltime - residual;
}

/**
 * @brief Dump profiling statistiscs.
 */
void profile_dump(void)
{
	fprintf(stderr, "Total Time: %lf\n", totaltime);
}
