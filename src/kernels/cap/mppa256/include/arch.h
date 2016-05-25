/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <arch.h> - Archtiecture contants and definitions.
 */

#ifndef ARCH_H_
#define ARCH_H_

	#include <mppaipc.h>
	
	/* Maximum cluster workload. */
	#define CLUSTER_WORKLOAD 0xfffff /* 1 MB */

	/*
	 * Number of clusters on a MPPA-256 processor.
	 */
	#define NUM_CLUSTERS 16

	/*
	 * Number of IO cores per IO cluster.
	 */
	#define NUM_IO_CORES 4

	/*
	 * Number of cores on a MPPA-256 processor.
	 */
	#define NUM_CORES 256
	
	/*
	 * Frequency of cores.
	 */
	#define MPPA_FREQUENCY 400

#endif /* ARCH_H_ */
