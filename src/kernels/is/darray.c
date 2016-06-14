/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <util.h>
#include <darray.h>

/**
 * @brief Creates a dynamic array.
 * 
 * @param size Maximum size.
 * 
 * @returns A dynamic array.
 */
struct darray *darray_create(long size)
{
	struct darray *da;
	
	/* Sanity check. */
	assert(size > 0);
	
	da = smalloc(sizeof(struct darray));
	
	/* Initialize array. */
	da->size = 0;
	da->maxsize = size;
	da->elements = scalloc(size, sizeof(long));
	
	return (da);
}

/**
 * @brief Destroys a dynamic array.
 * 
 * @param da Target dynamic array.
 */
void darray_destroy(struct darray *da)
{
	/* Sanity check. */
	assert(da != NULL);
	
	free(da->elements);
	free(da);
}

/**
 * @brief Expands a dynamic array.
 * 
 * @param da Target dynamic array.
 */
static void darray_expand(struct darray *da)
{	
	da->maxsize <<= 1;
	da->elements = srealloc(da->elements, sizeof(long)*da->maxsize);
}

/**
 * @brief Appends a value to a dynamic array.
 * 
 * @param da    Target dynamic array.
 * @param value Value.
 */
void darray_append(struct darray *da, int value)
{
	/* Sanity check. */
	assert(da != NULL);
	
	/* Expand array. */
	if (da->size == da->maxsize)
		darray_expand(da);
		
	da->elements[da->size++] = value;
}

/**
 * @brief Gets a value from a dynamic array.
 * 
 * @param da  Target dynamic array.
 * @param idx Index in dynamic array. 
 * 
 * @returns The value stored in @p idx of the dynamic array pointed to by @p da.
 */
long darray_get(struct darray *da, long idx)
{
	/* Sanity check. */
	assert(da != NULL);
	
	if (idx >= da->size || idx < 0)
		error("darray_get() out of bounds");
  
  return (da->elements[idx]);
}

/**
 * @brief Sets a value in a dynamic array.
 * 
 * @param da    Target dynamic array.
 * @param idx   Index in dynamic array.
 * @param value Value to store.
 */
void darray_set(struct darray *da, long idx, int value)
{
	/* Sanity check. */
	assert(da != NULL);
	
	if (idx < 0)
		error("darray_set() out of bounds");
	
	/* Expand array. */
	while (idx >= da->size++)
		darray_append(da, 0);

	da->elements[idx] = value;
}

