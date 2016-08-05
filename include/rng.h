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

#ifndef _COMMON_H_
#define _COMMON_H_
	
	#include <stddef.h>
	
	/**
	 * @brief Number of supported probability density functions.
	 */
	#define NR_PDFS 4

	/**
	 * @brief Supported probability density functions.
	 */
	/**@{*/
	#define RNG_BETA     1 /**< Beta.     */
	#define RNG_GAMMA    2 /**< Gamma.    */
	#define RNG_GAUSSIAN 3 /**< Gaussian. */
	#define RNG_POISSON  4 /**< Poisson.  */
	/**@}*/

	
	/* Forward definitions. */
	extern double *beta(int, double);
	extern double *gamma(int, double);
	extern double *gaussian(int, double);
	extern double *poisson(int, double);
	extern int *isrng(int, long *, int);

#endif /* _COMMON_H_ */
