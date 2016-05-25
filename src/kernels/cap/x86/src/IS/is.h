/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * is.h - Integer Sort kernel library.
 */

#ifndef _IS_H_
#define _IS_H_

	/*
	 * Dynamic array.
	 */
	struct darray
	{
		int size;      /* Current size. */
		int maxsize;   /* Maximum size. */
		int *elements; /* Elements.     */
	};

	/*
	 * Appends a value to a dynamic array.
	 */
	extern void darray_append(struct darray *da, int value);

	/*
	 * Creates a dynamic array.
	 */
	extern struct darray *darray_create(int size);
	
	/*
	 * Destroys a dynamic array.
	 */
	extern void darray_destroy(struct darray *da);
	
	/*
	 * Gets a value from a dynamic array.
	 */
	extern int darray_get(struct darray *da, int idx);
	
	/*
	 * Sets a value in a dynamic array.
	 */
	extern void darray_set(struct darray *da, int idx, int value);
	
	/*
	 * Returns the size of a dynamic array,
	 */
	#define darray_size(da) \
		((da)->size)

	/*
	 * Bucket sort algorithm.
	 */
	extern void integer_sort(int *array, int n);
	
	/*
	 * Sorts an array of numbers.
	 */
	extern void sort(struct darray *da);

#endif /* _IS_H_ */
