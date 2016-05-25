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
#include <limits.h>

#include "util.h"

/*
 * Priority queue.
 */
struct pqueue
{
	int size;
	int maxsize;
	int *objects;
	int *priorities;
};

/*
 * Fixes the heap property top-down. 
 */
static void pqueue_downheap(struct pqueue *p, int k)
{
	int prio; /* Working priority. */
	int obj;  /* Working object.   */
	
	prio = p->priorities[k];
	obj = p->objects[k];
	
	/* Traverse heap. */
	while (k <= p->size >> 1)
	{
		int j;
		
		j = k << 1;
		
		if (j < p->size)
		{
			if (p->priorities[j] > p->priorities[j + 1])
				j++;
		}
		
		if (prio <= p->priorities[j])
			break;
		
		p->priorities[k] = p->priorities[j];
		p->objects[k] = p->objects[j];
		
		k = j;
	}
	
	p->priorities[k] = prio;
	p->objects[k] = obj;
}

/*
 * Fixes the heap property bottom-up.
 */
static void pqueue_upheap(struct pqueue *p, int k)
{
	int prio;     /* Working priority. */
	int obj; /* Working object.   */
	
	prio = p->priorities[k];
	obj = p->objects[k];
	p->priorities[0] = INT_MIN;
	
	/* Traverse heap. */
	while (p->priorities[k >> 1] >= prio)
	{
		p->priorities[k] = p->priorities[k >> 1];
		p->objects[k] = p->objects[k >> 1];
		k >>= 1;
	}
	
	p->priorities[k] = prio;
	p->objects[k] = obj;
}

/*
 * Creates a priority queue.
 */
struct pqueue *pqueue_create(int maxsize)
{
	struct pqueue *p;
	
	assert(maxsize > 0);
	
	/* Allocate structures. */
	p = smalloc(sizeof(struct pqueue));
	p->objects = smalloc((maxsize + 1)*sizeof(int));
	p->priorities = smalloc((maxsize + 1)*sizeof(int));
	
	/* Initialize priority queue. */
	p->size = 0;
	p->maxsize = maxsize;
	
	return (p);
}

/*
 * Destroys a priority queue.
 */
void pqueue_destroy(struct pqueue *p)
{
	assert(p != NULL);
	
	free(p->priorities);
	free(p->objects);
	free(p);
}

/*
 * Asserts if a priority queue is full.
 */
int pqueue_full(struct pqueue *p)
{
	assert(p != NULL);
	
	return (p->size == p->maxsize);
}

/*
 * Asserts if a priority queue is empty.
 */
int pqueue_empty(struct pqueue *p)
{
	assert(p != NULL);
	
	return (p->size == 0);
}

/*
 * Changes the priority of an object in a priority queue.
 */
int pqueue_change(struct pqueue *p, int obj, int newprio)
{
	int i;       /* Item index.   */
	int oldprio; /* Old priority. */
	
	/* Sanity check. */
	assert(p != NULL);
	assert(newprio >= 0);

	/* Search for item. */
	for (i = 1; i <= p->size; i++)
	{	
		/* Found. */
		if (p->objects[i] == obj)
			goto found;
	}
	
	return (-1);
	
found:

	/* Reassign priority. */
	oldprio = p->priorities[i];
	p->priorities[i] = newprio;
	if (newprio < oldprio)
		pqueue_upheap(p, i);
	else if (newprio > oldprio)
		pqueue_downheap(p, i);
	
	return (oldprio);
}

/*
 * Inserts an object in a priority queue.
 */
void pqueue_insert(struct pqueue *p, int obj, int prio)
{
	int i;
	
	/* Sanity check. */
	assert(p != NULL);
	assert(prio >= 0);
	
	/* Priority queue is full. */
	if (pqueue_full(p))
		error("priority queue is full");
	
	i = ++p->size;
	
	p->objects[i] = obj;
	p->priorities[i] = prio;
	
	pqueue_upheap(p, i);
}

/*
 * Removes an object from a priority queue.
 */
int pqueue_remove(struct pqueue *p)
{
	int i;   /* Priority queue index. */
	int obj; /* Object.               */
	
	/* Sanity check. */
	assert(p != NULL);
	
	/* Priority queue is empty. */
	if (pqueue_empty(p))
		error("priority queue is empty");
	
	obj = p->objects[1];
	i = p->size--;
	
	p->objects[1] = p->objects[i];
	p->priorities[1] = p->priorities[i];
	
	pqueue_downheap(p, 1);
	
	return (obj);
}
