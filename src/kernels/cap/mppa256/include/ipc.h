/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef IPC_H_
#define IPC_H_

#ifdef _MASTER_
	extern int infd[];  /* Input channels.  */
	extern int outfd[]; /* Output channels. */
#else
	extern int rank;  /* Process rank.   */
	extern int infd;  /* Input channel.  */
	extern int outfd; /* Output channel. */
#endif

#ifdef _MASTER_
	extern void spawn_slaves(void);
	extern void join_slaves(void);
#endif
	extern void open_noc_connectors(void);
	extern void close_noc_connectors(void);

#endif /* IPC_H_ */
