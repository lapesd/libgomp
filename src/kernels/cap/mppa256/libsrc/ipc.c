/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>
#include <mppaipc.h>

#include <arch.h>
#include <global.h>

#ifdef _MASTER_

/* Interprocess communication. */
int infd[NUM_CLUSTERS];               /* Input channels.  */
int outfd[NUM_CLUSTERS];              /* Output channels. */
static mppa_pid_t pids[NUM_CLUSTERS]; /* Processes IDs.   */

/*
 * Spwans slave processes.
 */
void spawn_slaves(void)
{
	int i;          /* Loop index. */
	char arg0[4];   /* Argument 0. */
	char *args[2];  /* Arguments.  */

	/* Spawn slaves. */
	args[1] = NULL;
	for (i = 0; i < nclusters; i++)
	{	
		sprintf(arg0, "%d", i);
		args[0] = arg0;
		pids[i] = mppa_spawn(i, NULL, "slave", (const char **)args, NULL);
		assert(pids[i] != -1);
	}
}

/*
 * Joins slave processes.
 */
void join_slaves(void)
{
	int i;
	
	/* Join slaves. */
	for (i = 0; i < nclusters; i++)
	{
		data_receive(infd[i], &slave[i], sizeof(uint64_t));
		mppa_waitpid(pids[i], NULL, 0);
	}
}

/*
 * Open NoC connectors.
 */
void open_noc_connectors(void)
{
	int i;          /* Loop index.     */
	char path[35];  /* Connector path. */

	/* Open channels. */
	for (i = 0; i < nclusters; i++)
	{		
		sprintf(path, "/mppa/channel/%d:%d/128:%d", i, i + 17, i + 17);
		outfd[i] = mppa_open(path, O_WRONLY);
		assert(outfd[i] != -1);
		
		sprintf(path, "/mppa/channel/128:%d/%d:%d", i + 33, i, i + 33);
		infd[i] = mppa_open(path, O_RDONLY);
		assert(outfd[i] != -1);
	}
}

/*
 * Close NoC connectors.
 */
void close_noc_connectors(void)
{
	int i;
	
	/* Close channels. */
	for (i = 0; i < nclusters; i++)
	{
		mppa_close(outfd[i]);
		mppa_close(infd[i]);
	}
}

#else

/* Inter process communication. */
int rank;  /* Process rank.   */
int infd;  /* Input channel.  */
int outfd; /* Output channel. */

/*
 * Opens NoC connectors.
 */
void open_noc_connectors(void)
{
	char path[35];
	
	/* Open input channel. */
	sprintf(path, "/mppa/channel/%d:%d/128:%d", rank, rank + 17, rank + 17);
	infd = mppa_open(path, O_RDONLY);
	assert(infd != -1);
	sprintf(path, "/mppa/channel/128:%d/%d:%d", rank + 33, rank, rank + 33);
	outfd = mppa_open(path, O_WRONLY);
	assert(outfd != -1);
}

/*
 * Closes NoC connectors.
 */
void close_noc_connectors(void)
{
	/* Close channels. */
	mppa_close(infd);
	mppa_close(outfd);
}

#endif
