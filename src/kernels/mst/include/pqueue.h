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
 
 #ifndef _PQUEUE_H_
 #define _PQUEUE_H_
 
	
	extern struct pqueue *pqueue_create(int);	
	extern void pqueue_destroy(struct pqueue *);
	extern int pqueue_empty(struct pqueue *);
	extern int pqueue_full(struct pqueue *);
	extern int pqueue_change(struct pqueue *, int, double);
	extern void pqueue_insert(struct pqueue *, int, double);
	extern int pqueue_remove(struct pqueue *);
 
 #endif /* _PQUEUE_H_ */
