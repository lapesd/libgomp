/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa.c - MPPA interface implementation.
 */

#include <stdint.h>
#include <mppa/osconfig.h>

uint64_t k1_io_read64(unsigned addr)
{
#ifdef BUGGED_MPPA
	return (__k1_io_read64((void *)addr));
#else
	return (0);
#endif
}

int k1_get_cluster_id(void)
{
	return (__k1_get_cluster_id());
}

void k1_dcache_invalidate_mem_area(void *p, size_t n)
{
	__k1_dcache_invalidate_mem_area((__k1_uintptr_t)p, n);
}
