/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef MASTER_H_
#define MASTER_H_

	#include <arch.h>

	/*
	 * Close NoC connectors.
	 */
	extern void close_noc_connectors(void);

	/*
	 * Joins slave processes.
	 */
	extern void join_slaves(void);
	
	/*
	 * Kills slave processes.
	 */
	extern void kill_slaves(void);
	
	/*
	 * Open NoC connectors.
	 */
	extern void open_noc_connectors(void);

	/*
	 * Spawns slave processes.
	 */
	extern void spawn_slaves(void);
	
	/*
	 * Waits for slaves to be ready.
	 */
	extern void sync_slaves(void);
	
	
	/* Channels. */
	extern int infd[NUM_CLUSTERS];  /* Input channels.  */
	extern int outfd[NUM_CLUSTERS]; /* Output channels. */

#endif /* MASTER_H_ */
