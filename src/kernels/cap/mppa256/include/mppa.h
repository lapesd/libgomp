/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa.h - MPPA interface.
 */

#ifndef MPPA_H_
#define MPPA_H_

	#include <stdlib.h>
	#include <stdint.h>

	extern int k1_get_cluster_id(void);
	extern void k1_dcache_invalidate_mem_area(void *, size_t);
	extern uint64_t k1_io_read64(unsigned addr);

#endif /* MPPA_H_ */
