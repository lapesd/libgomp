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

#ifndef _IS_H_
#define _IS_H_
	
	#include <rng.h>
	
	/**
	 * @brief Default parameters for IS kernel.
	 */
	/**@{*/
	#define IS_NNUMBERS 2147483648L /**< Number of numbers to sort.          */
	#define IS_NCLASSES 32          /**< Number of classes.                  */
	#define IS_PDF      RNG_BETA    /**< Input probability density function. */
	/**@}*/
	
	/* Forward definitions. */
	extern void integer_sort(int *, long);

#endif /* _IS_H_ */
