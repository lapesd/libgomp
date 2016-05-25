/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

	#include <stdlib.h>

	/* Message types. */
	#define DIE          0 /* Die.                */
	#define SORTWORK     1 /* Sort array.         */
	#define SORTRESULT   2 /* Sort array result.  */
	#define FINDWORK     3 /* Find pivot element. */
	#define FINDRESULT   4 /* Find pivot element. */
	#define REDUCTWORK   5 /* Row reduction.      */
	#define REDUCTRESULT 6 /* Row reduction.      */
	
	/*
	 * Message.
	 */
	struct message
	{
		int type; /* Message type (see above). */
		
		union
		{
			/* SORTWORK. */
			struct 
			{
				int id;   /* Bucket ID.       */
				int size; /* Minibucket size. */
			} sortwork;
			
			/* SORTRESULT. */
			struct
			{
				int id;   /* Bucket ID.       */
				int size; /* Minibucket size. */
			} sortresult;
			
			/* FINDWORK. */
			struct 
			{
				int i0, j0; /* Block start.  */
				int height; /* Block height. */
				int width;  /* Block width.  */
			} findwork;
			
			/* FINDRESULT. */
			struct
			{
				int ipvt;    /* ith index of pivot. */
				int jpvt;    /* jth index of pivot. */
				int i0, j0;  /* Block start.        */
			} findresult;
			
			/* REDUCTWORK. */
			struct
			{
				int ipvt;   /* Row index of pivot. */
				int i0, j0; /* Block start.        */
				int height; /* Block height.       */
				int width;  /* Block width.        */
			} reductwork;
			
			/* REDUCTRESULT. */
			struct
			{
				int i0, j0; /* Block start.  */
				int height; /* Block height. */
				int width;  /* Block width.  */
				
			} reductresult;
		} u;
		
		/* Next message of a list. */
		struct message *next;
	};
	
	/*
	 * Creates a message.
	 */
	extern struct message *message_create(int type, ...);
	
	/*
	 * Destroys a message.
	 */
	extern void message_destroy(struct message *msg);
	
	/*
	 * Receives a message.
	 */
	extern struct message *message_receive(int infd);
	
	/*
	 * Sends a message.
	 */
	extern void message_send(int outfd, struct message *msg);
	
	/*
	 * Asserts if a list is empty.
	 */
	#define empty(l)  \
		((l) == NULL) \

	/*
	 * Pushes a message on a list.
	 */
	#define push(l, msg)                \
		{ (msg)->next = (l); (l) = msg; } \

	/*
	 * Pops a message from a list.
	 */
	#define pop(l, msg)                   \
		{ (msg) = (l); (l) = (msg)->next; } \

#endif /* MESSAGE_H_ */
