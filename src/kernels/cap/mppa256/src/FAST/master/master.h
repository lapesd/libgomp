/*
 * Copyright(C) 2014	 Alyson D. Pereira <alyson.deives@outlook.com>, 
 * 			 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * master.h -  Private master library.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

	#include <arch.h>
	#include <stdlib.h>
	
	/*
	 * Maximum thread num per cluster.
	 */
	#define MAX_THREADS (16)
	
	/*
	 * Maximum chunk size.
	 */
	#define CHUNK_SIZE (512)
	
	/*
	 * Maximum mask size.
	 */
	#define MASK_SIZE (54)
	
	/*
	 * Mask radius.
	 */
	#define MASK_RADIUS (3)
	
	/*
	 * Maximum image size.
	 */
	#define IMG_SIZE (24576)
	
	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0
	
#endif /* _MASTER_H_ */
