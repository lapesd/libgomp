/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef SLAVE_H_
#define SLAVE_H_

	/*
	 * Opens NoC connectors.
	 */
	extern void open_noc_connectors(void);
	
	/*
	 * Synchronizes with master process.
	 */
	extern void sync_master(void);
	
	/*
	 * Closes NoC connectors.
	 */
	extern void close_noc_connectors(void);

	extern int rank;    /* Process rank.         */
	extern int infd;    /* Input channel.        */
	extern int outfd;  /* Output channel.       */

#endif /* SLAVE_H_ */
