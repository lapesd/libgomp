/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.h -  Private slave library.
 */

#ifndef _SLAVE_H_
#define _SLAVE_H_

	#include <stdlib.h>
	
	/*
	 * pi
	 */
	#define PI 3.14159265359
	
	/*
	 * e
	 */
	#define E 2.71828182845904
	
	/*
	 * Standard deviation.
	 */
	#define SD 0.8
	
	/*
	 * Maximum chunk size.
	 */
	#define CHUNK_SIZE (1024)
	
	/*
	 * Maximum mask size.
	 */
	#define MASK_SIZE (20)
	
	/*
	 * Maximum image size.
	 */
	#define IMG_SIZE (32768)
	
	/* Type of messages. */
	#define MSG_CHUNK 1
	#define MSG_DIE   0

	/*
	 * Synchronizes with master process.
	 */
	extern void sync_master(void);
	
	/*
	 * Opens NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Closes NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/* Inter process communication. */
	extern int rank;  /* Process rank.   */
	extern int infd;  /* Input channel.  */
	extern int outfd; /* Output channel. */
	
#endif /* _SLAVE_H_ */
