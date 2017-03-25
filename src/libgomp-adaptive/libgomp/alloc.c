/* Copyright (C) 2005, 2009 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@redhat.com>.

   This file is part of the GNU OpenMP Library (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This file contains wrappers for the system allocation routines.  Most
   places in the OpenMP API do not make any provision for failure, so in
   general we cannot allow memory allocation to fail.  */

#include "libgomp.h"
#include <stdlib.h>
#if defined(LIBGOMP_USE_NUMA)
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <numa.h>
#include <numaif.h>
#include <stdio.h>
#endif

#if defined(LIBGOMP_FORCE_ALIGNED)
#include <strings.h>
#endif

void *
gomp_malloc (size_t size)
{
#if defined(LIBGOMP_FORCE_ALIGNED)
  void *ret;
  posix_memalign( &ret, 64, size );
#else
  void *ret = malloc (size);
#endif
  if (ret == NULL)
    gomp_fatal ("Out of memory allocating %lu bytes", (unsigned long) size);
  return ret;
}

void *
gomp_malloc_cleared (size_t size)
{
#if defined(LIBGOMP_FORCE_ALIGNED)
  void *ret;
  posix_memalign( &ret, 64, size );
  bzero( ret, size );
#else
  void *ret = calloc (1, size);
#endif
  if (ret == NULL)
    gomp_fatal ("Out of memory allocating %lu bytes", (unsigned long) size);
  return ret;
}

void *
gomp_realloc (void *old, size_t size)
{
  void *ret = realloc (old, size);
  if (ret == NULL)
    gomp_fatal ("Out of memory allocating %lu bytes", (unsigned long) size);
  return ret;
}


void* omp_locality_domain_alloc(size_t size)
{
#if !defined(LIBGOMP_USE_NUMA)
  return gomp_malloc(size);
#else
  void* retval;
  size_t pagesize, size_mmap;
  
  pagesize = getpagesize();
  size_mmap = (( size + pagesize -1 ) / pagesize) * pagesize + pagesize;
  retval = mmap( 0, size_mmap, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, (off_t)0 );
  if (retval == (void*)-1)
    return 0;
  return retval;
#endif
}

int omp_locality_domain_free(void* ptr, size_t size)
{
#if !defined(LIBGOMP_USE_NUMA)
  free(ptr);
  return 0;
#else
  size_t pagesize, size_mmap;
  pagesize = getpagesize();

  if (((uintptr_t)ptr & (pagesize-1)) !=0) /* should aligned on page boundary */
  {
    errno = EFAULT;
    return -1;
  }
  size_mmap = (( size + pagesize -1 ) / pagesize) * pagesize + pagesize;
  return munmap( ptr, size_mmap );
#endif
}

#if defined(LIBGOMP_USE_NUMA)
static inline int kaapi_bitmap_value_first1_and_zero_64( uint64_t* b )
{
  /* Note: for WIN32, to have a look at _BitScanForward */
  int fb = __builtin_ffsl( *b );
  if (fb ==0) return 0;
  *b &= ~( ((uint64_t)1) << (fb-1) );
  return fb;
}
#endif

#if defined(LIBGOMP_USE_NUMA)
static int _omp_locality_domain_bind_bloc1dcyclic(uint64_t ldset, const void* addr, size_t size, size_t blocsize) 
{
  const int mode = MPOL_BIND;
  const unsigned int flags = MPOL_MF_STRICT | MPOL_MF_MOVE;
  const char* base = (const char*)addr;
  uint64_t numaset;

  size_t pagesize = getpagesize();
  const unsigned long maxnode = 32;
  unsigned long nodemask
      [(32 + (8 * sizeof(unsigned long) -1))/ (8 * sizeof(unsigned long))];

  if (((uintptr_t)addr & (pagesize-1)) !=0) /* should aligned on page boundary */
  {
    errno = EFAULT;
    return -1;
  }

  if ((blocsize & (pagesize-1)) !=0) /* should be divisible by getpagesize() */
  {
    blocsize &= ~(blocsize & (pagesize-1));
  }

  numaset =0;
  int node = 0;
  while (size >0)
  {
    int ldid = kaapi_bitmap_value_first1_and_zero_64( &numaset );
    if (ldid ==0)
    {
      numaset = ldset;
      continue;
    }
    --ldid;
    node = ldid;

    memset(nodemask, 0, sizeof(nodemask));
    nodemask[node / (8 * sizeof(unsigned long))] |= 1UL << (node % (8 * sizeof(unsigned long)));

    if (mbind((void*)base, blocsize, mode, nodemask, maxnode, flags))
      return -1;
printf("Bind @=[%p, %li) on node:%i\n", (void*)base, (long)blocsize, node );

    base += blocsize;
    if (size > blocsize)
      size -= blocsize;
    else
      size = 0;
  }
  /* add mbind for an extra page */
  if (mbind((void*)base, pagesize, mode, nodemask, maxnode, flags))
      return -1;
  return 0;
}
#endif


int omp_locality_domain_bind_bloc1dcyclic(const void* addr, size_t size, size_t blocsize) 
{
#if !defined(LIBGOMP_USE_NUMA)
  return 0;
#else
  struct gomp_thread_pool *pool;
  struct gomp_thread* thr;
  uint64_t ldset;
  int i;
  
  ldset =0;
  thr = gomp_thread ();
  pool = thr->thread_pool;
  for (i=0; i<32; ++i)
  {
    if (pool->numa_info[i].size >0)
      ldset |= 1 << i;
  }
  return _omp_locality_domain_bind_bloc1dcyclic(ldset, addr, size, blocsize );
#endif
}


void* omp_locality_domain_allocate_bloc1dcyclic(size_t size, size_t blocsize) 
{
#if !defined(LIBGOMP_USE_NUMA)
  return gomp_malloc(size);
#else
  int err;
  void* retval;

  retval = omp_locality_domain_alloc(size);
  if (retval == 0)
    return 0;

  err = omp_locality_domain_bind_bloc1dcyclic( retval, size, blocsize );
  if (err !=0)
  {
    omp_locality_domain_free( retval, size );
    retval = 0;
  }
  return retval;
#endif
}

int omp_locality_domain_bind_bloc1d(const void* addr, size_t size) 
{
#if !defined(LIBGOMP_USE_NUMA)
  return 0;
#else
  struct gomp_thread* thr;
  size_t count = 0;

  thr = gomp_thread ();
  count = thr->ts.team->nnumanodes;
  if (count ==0) return 0;

  return omp_locality_domain_bind_bloc1dcyclic( addr, size, size < count ? size : size / count );
#endif
}


void* omp_locality_domain_allocate_bloc1d(size_t size) 
{
#if !defined(LIBGOMP_USE_NUMA)
  return gomp_malloc(size);
#else
  struct gomp_thread* thr;
  size_t count = 0;

  thr = gomp_thread ();
  count = thr->ts.team->nnumanodes;
  if (count ==0) return 0;

  return omp_locality_domain_allocate_bloc1dcyclic( size, size < count ? size : size / count );
#endif
}

