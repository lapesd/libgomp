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

#ifndef _DARRAY_H_
#define _DARRAY_H_
	
	#include <assert.h>
	
	/**
	 * @brief Dynamic array.
	 */
	struct darray
	{
		long size;     /**< Current size. */
		long maxsize;  /**< Maximum size. */
		int *elements; /**< Elements.     */
	};
	
	/**
	 * @brief Opaque pointer to a dynamic array.
	 */
	typedef struct darray * darray_tt;

	/* Forward definitions. */
	extern void darray_append(darray_tt, int);
	extern struct darray *darray_create(long);
	extern void darray_destroy(darray_tt);
	extern long darray_get(darray_tt, long);
	extern void darray_set(darray_tt, long, int);
	
	/**
	 * @brief Returns the size of a dynamic array.
	 * 
	 * @param da Target array.
	 */
	inline long darray_size(darray_tt da)
	{
		assert(da != NULL);
		
		return (da->size);
	}

#endif /* _DARRAY_H_ */
