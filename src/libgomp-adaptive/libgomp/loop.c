/* Copyright (C) 2005, 2008, 2009 Free Software Foundation, Inc.
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

/* This file handles the LOOP (FOR/DO) construct.  */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <sched.h>
#include <limits.h>
#include <stdlib.h>
#include "libgomp.h"

#include <stdio.h>

#if defined(LIBGOMP_USE_ADAPTIVE)

static void
gomp_loop_adaptive_init_master (struct gomp_work_share *ws, long start, long end, long chunk_size) 
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;
  long i, nthreads = team ? team->nthreads : 1;

  ws->nb_iterations_left = end - start;

  for (i=0; i<nthreads; ++i)
  {
    struct gomp_ws_adaptive_chunk *ws_chunk = &ws->adaptive_chunks[i];
    ws_chunk->is_init = false;
  }
 //printf("------- Master Thread: %li / #:%li ws:%p coreid: %i -> [%li, %li)\n", i, nthreads, (void*)ws, thr->coreid, start, end);

//printf("I'm master thread %i in adaptive loop init\n", gomp_thread()->ts.team_id);
#if defined(LIBGOMP_USE_NUMA)
#if 0
  struct gomp_thread *thr = gomp_thread();
  struct gomp_team *team = thr->ts.team;
  int j;
  printf("Used #numa nodes: %i\n", team->nnumanodes);
  struct gomp_thread_pool *pool = thr->thread_pool;
  for (j=0; j<32; ++j)
  {
    if (pool->numa_info[j].size >0)
    {
      printf("Node: %i, prefix: %i,  #threads:%i  ", j, team->startindex[j], pool->numa_info[j].size); 
      int k;
      for (k=0; k<pool->numa_info[j].size; ++k)
        printf("%i ", pool->numa_info[j].team_ids[k]);
      printf("\n");
    }
  }
#endif
#if 0 // DEAD CODE
  struct gomp_task_icv* icv = gomp_icv(false);
  if (1) //icv->attr_distribute)
  {
    int i;
    ws->attr_cpuset = icv->attr_distribute;
    CPU_ZERO( &ws->attr_cpuset);
    /* iterate over all threads in the pool to get the cpuset mask */
    struct gomp_thread_pool *pool = thr->thread_pool;
    for (i=0; i< pool->threads_used; ++i)
    {
      struct gomp_thread* thi = pool->threads[i];
      if (thi ==0) thi = thr;
      if (thi !=0)
      {
        CPU_SET( thi->coreid, &ws->cpuset );
      }
    }
  }
#endif
#endif
}

void
gomp_loop_adaptive_init_worker (
  struct gomp_work_share *ws,
  struct gomp_thread *thr,
  long start, long end, long chunk_size
) 
{
  struct gomp_team *team = thr->ts.team;
  int i = thr->ts.team_id;
  struct gomp_ws_adaptive_chunk *current_chunk = &ws->adaptive_chunks[i];
  int nthreads = team ? team->nthreads : 1;
  long range = (end - start) / nthreads;

  if (current_chunk->is_init)
      abort ();

  //printf("I'm worker thread %i in adaptive loop init\n", i);
  
  current_chunk->nb_exec             = 0;
#if defined(LIBGOMP_USE_NUMA)
  /* chunk must be refill in critical section, if steal can occurs  */
  kaapi_atomic_lock (&current_chunk->lock);
  current_chunk->begin               = start + (team->startindex[thr->numaid] + thr->index_numanode) * range;
  current_chunk->end                 = current_chunk->begin + range;
  if (current_chunk->end > end) 
    current_chunk->end = end; 
  if (team->startindex[thr->numaid] + thr->index_numanode == nthreads-1)
    current_chunk->end = end; 
  kaapi_atomic_unlock (&current_chunk->lock);
#else
  /* chunk must be refill in critical section, if steal can occurs  */
  kaapi_atomic_lock (&current_chunk->lock);
  current_chunk->begin               = start + (i * range);
  current_chunk->end                 = (i == nthreads - 1) ? end : start + ((i + 1) * range);
  kaapi_atomic_unlock (&current_chunk->lock);
#endif
 //printf("Thread: %i ws:%p coreid: %i -> [%li, %li)\n", i, (void*)ws, thr->coreid, current_chunk->begin, current_chunk->end);
 current_chunk->is_init = true;
}

static void
gomp_loop_adaptive_cleanup (void) {

  /* François ? do nothing because at the end of a previous chunk computation, we always have end <= begin */
#if 1
  struct gomp_thread *thr = gomp_thread();
  struct gomp_ws_adaptive_chunk *current_chunk = &thr->ts.work_share->adaptive_chunks[thr->ts.team_id];
  current_chunk->is_init             = false;
  current_chunk->nb_exec             = 0;
  kaapi_atomic_lock (&current_chunk->lock);
  current_chunk->begin               = 0;
  current_chunk->end                 = 0;
  kaapi_atomic_unlock (&current_chunk->lock);
#endif
}

#endif // if defined(LIBGOMP_USE_ADAPTIVE)

/* Initialize the given work share construct from the given arguments.  */

static inline void
gomp_loop_init (struct gomp_work_share *ws, long start, long end, long incr,
		enum gomp_schedule_type sched, long chunk_size

)
{
  ws->sched = sched;
  ws->chunk_size = chunk_size;
  /* Canonicalize loops that have zero iterations to ->next == ->end.  */
  ws->end = ((incr > 0 && start > end) || (incr < 0 && start < end))
	    ? start : end;
  ws->incr = incr;
#if defined(LIBGOMP_USE_ADAPTIVE)
  ws->start_t0 = start;
#endif
  ws->next = start;
  if (sched == GFS_DYNAMIC)
    {
      ws->chunk_size *= incr;

#ifdef HAVE_SYNC_BUILTINS
      {
	/* For dynamic scheduling prepare things to make each iteration
	   faster.  */
	struct gomp_thread *thr = gomp_thread ();
	struct gomp_team *team = thr->ts.team;
	long nthreads = team ? team->nthreads : 1;

	if (__builtin_expect (incr > 0, 1))
	  {
	    /* Cheap overflow protection.  */
	    if (__builtin_expect ((nthreads | ws->chunk_size)
				  >= 1UL << (sizeof (long)
					     * __CHAR_BIT__ / 2 - 1), 0))
	      ws->mode = 0;
	    else
	      ws->mode = ws->end < (LONG_MAX
				    - (nthreads + 1) * ws->chunk_size);
	  }
	/* Cheap overflow protection.  */
	else if (__builtin_expect ((nthreads | -ws->chunk_size)
				   >= 1UL << (sizeof (long)
					      * __CHAR_BIT__ / 2 - 1), 0))
	  ws->mode = 0;
	else
	  ws->mode = ws->end > (nthreads + 1) * -ws->chunk_size - LONG_MAX;
      }
#endif
    }
  
#if defined(LIBGOMP_USE_ADAPTIVE)
  if (sched == GFS_ADAPTIVE)
  {
    gomp_loop_adaptive_init_master(ws, start, end, chunk_size);
  }
#endif
}

/* The *_start routines are called when first encountering a loop construct
   that is not bound directly to a parallel construct.  The first thread 
   that arrives will create the work-share construct; subsequent threads
   will see the construct exists and allocate work from it.

   START, END, INCR are the bounds of the loop; due to the restrictions of
   OpenMP, these values must be the same in every thread.  This is not 
   verified (nor is it entirely verifiable, since START is not necessarily
   retained intact in the work-share data structure).  CHUNK_SIZE is the
   scheduling parameter; again this must be identical in all threads.

   Returns true if there's any work for this thread to perform.  If so,
   *ISTART and *IEND are filled with the bounds of the iteration block
   allocated to this thread.  Returns false if all work was assigned to
   other threads prior to this thread's arrival.  */

static bool
gomp_loop_static_start (long start, long end, long incr, long chunk_size,
			long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();

#if defined(LIBGOMP_PROFILE_LOOP)
  union tick_t t0,t1;
  _GET_TICK(t0);
#endif

  thr->ts.static_trip = 0;
  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_STATIC, chunk_size);
      gomp_work_share_init_done ();
    }
#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tinitloop += t1.tick - t0.tick;
#endif

  bool ret = !gomp_iter_static_next (istart, iend);
#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t0);
  PF(thr)->tpop += t0.tick - t1.tick;
  if (ret)
    PF(thr)->startcomp = t0.tick;
  else
    PF(thr)->startcomp = 0;
#endif
  return ret;
}

static bool
gomp_loop_dynamic_start (long start, long end, long incr, long chunk_size,
			 long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

#if defined(LIBGOMP_PROFILE_LOOP)
  union tick_t t0,t1;
  _GET_TICK(t0);
#endif

  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_DYNAMIC, chunk_size);
      gomp_work_share_init_done ();
    }

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tinitloop += t1.tick - t0.tick;
#endif

#ifdef HAVE_SYNC_BUILTINS
  ret = gomp_iter_dynamic_next (istart, iend);
#else
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_dynamic_next_locked (istart, iend);
  gomp_mutex_unlock (&thr->ts.work_share->lock);
#endif

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t0);
  PF(thr)->tpop += t0.tick - t1.tick;
  if (ret)
    PF(thr)->startcomp = t0.tick;
  else
    PF(thr)->startcomp = 0;
#endif

  return ret;
}

static bool
gomp_loop_guided_start (long start, long end, long incr, long chunk_size,
			long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

#if defined(LIBGOMP_PROFILE_LOOP)
  union tick_t t0,t1;
  _GET_TICK(t0);
#endif

  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_GUIDED, chunk_size);
      gomp_work_share_init_done ();
    }
#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tinitloop += t1.tick - t0.tick;
#endif

#ifdef HAVE_SYNC_BUILTINS
  ret = gomp_iter_guided_next (istart, iend);
#else
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_guided_next_locked (istart, iend);
  gomp_mutex_unlock (&thr->ts.work_share->lock);
#endif

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t0);
  PF(thr)->tpop += t0.tick - t1.tick;
  if (ret)
    PF(thr)->startcomp = t0.tick;
  else
    PF(thr)->startcomp = 0;
#endif

  return ret;
}

#if defined(LIBGOMP_USE_ADAPTIVE)
static bool
gomp_loop_adaptive_start (long start, long end, long incr, long chunk_size,
			  long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();

#if defined(LIBGOMP_PROFILE_LOOP)
  union tick_t t0,t1;
  _GET_TICK(t0);
#endif

  if (gomp_work_share_start (false))
    {
//printf("Master: %i\n", thr->ts.team_id );
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_ADAPTIVE, chunk_size);
      gomp_work_share_init_done ();
    }

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tinitloop += t1.tick - t0.tick;
#endif
  return gomp_iter_adaptive_next (istart, iend);;
}
#endif


bool
GOMP_loop_runtime_start (long start, long end, long incr,
			 long *istart, long *iend)
{
  bool ret = false;
  struct gomp_task_icv *icv = gomp_icv (false);

#if defined(LIBGOMP_PROFILE_LOOP)
  struct gomp_thread *thr = gomp_thread ();
  if (PF(thr)->startcomp !=0) abort();
#endif

  switch (icv->run_sched_var)
    {
    case GFS_STATIC:
      ret = gomp_loop_static_start (start, end, incr, icv->run_sched_modifier,
				     istart, iend);
      break;
    case GFS_DYNAMIC:
      ret = gomp_loop_dynamic_start (start, end, incr, icv->run_sched_modifier,
				      istart, iend);
      break;
    case GFS_GUIDED:
      ret = gomp_loop_guided_start (start, end, incr, icv->run_sched_modifier,
				     istart, iend);
      break;
#if defined(LIBGOMP_USE_ADAPTIVE)
    case GFS_ADAPTIVE:
      ret = gomp_loop_adaptive_start (start, end, incr, icv->run_sched_modifier,
				     istart, iend);
      break;
#endif
    case GFS_AUTO:
      /* For now map to schedule(static), later on we could play with feedback
	 driven choice.  */
      ret = gomp_loop_static_start (start, end, incr, 0, istart, iend);
      break;

    default:
      abort ();
    }

  return ret;
}

/* The *_ordered_*_start routines are similar.  The only difference is that
   this work-share construct is initialized to expect an ORDERED section.  */

static bool
gomp_loop_ordered_static_start (long start, long end, long incr,
				long chunk_size, long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();

  thr->ts.static_trip = 0;
  if (gomp_work_share_start (true))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_STATIC, chunk_size);
      gomp_ordered_static_init ();
      gomp_work_share_init_done ();
    }

  return !gomp_iter_static_next (istart, iend);
}

static bool
gomp_loop_ordered_dynamic_start (long start, long end, long incr,
				 long chunk_size, long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  if (gomp_work_share_start (true))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_DYNAMIC, chunk_size);
      gomp_mutex_lock (&thr->ts.work_share->lock);
      gomp_work_share_init_done ();
    }
  else
    gomp_mutex_lock (&thr->ts.work_share->lock);

  ret = gomp_iter_dynamic_next_locked (istart, iend);
  if (ret)
    gomp_ordered_first ();
  gomp_mutex_unlock (&thr->ts.work_share->lock);

  return ret;
}

static bool
gomp_loop_ordered_guided_start (long start, long end, long incr,
				long chunk_size, long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  if (gomp_work_share_start (true))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_GUIDED, chunk_size);
      gomp_mutex_lock (&thr->ts.work_share->lock);
      gomp_work_share_init_done ();
    }
  else
    gomp_mutex_lock (&thr->ts.work_share->lock);

  ret = gomp_iter_guided_next_locked (istart, iend);
  if (ret)
    gomp_ordered_first ();
  gomp_mutex_unlock (&thr->ts.work_share->lock);

  return ret;
}

bool
GOMP_loop_ordered_runtime_start (long start, long end, long incr,
				 long *istart, long *iend)
{
  struct gomp_task_icv *icv = gomp_icv (false);
  switch (icv->run_sched_var)
    {
    case GFS_STATIC:
      return gomp_loop_ordered_static_start (start, end, incr,
					     icv->run_sched_modifier,
					     istart, iend);
    case GFS_DYNAMIC:
      return gomp_loop_ordered_dynamic_start (start, end, incr,
					      icv->run_sched_modifier,
					      istart, iend);
    case GFS_GUIDED:
      return gomp_loop_ordered_guided_start (start, end, incr,
					     icv->run_sched_modifier,
					     istart, iend);
    case GFS_AUTO:
      /* For now map to schedule(static), later on we could play with feedback
	 driven choice.  */
      return gomp_loop_ordered_static_start (start, end, incr,
					     0, istart, iend);
    default:
      abort ();
    }
}

/* The *_next routines are called when the thread completes processing of 
   the iteration block currently assigned to it.  If the work-share 
   construct is bound directly to a parallel construct, then the iteration
   bounds may have been set up before the parallel.  In which case, this
   may be the first iteration for the thread.

   Returns true if there is work remaining to be performed; *ISTART and
   *IEND are filled with a new iteration block.  Returns false if all work
   has been assigned.  */

static bool
gomp_loop_static_next (long *istart, long *iend)
{
  return !gomp_iter_static_next (istart, iend);
}

static bool
gomp_loop_dynamic_next (long *istart, long *iend)
{
  bool ret;

#ifdef HAVE_SYNC_BUILTINS
  ret = gomp_iter_dynamic_next (istart, iend);
#else
  struct gomp_thread *thr = gomp_thread ();
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_dynamic_next_locked (istart, iend);
  gomp_mutex_unlock (&thr->ts.work_share->lock);
#endif

  return ret;
}

static bool
gomp_loop_guided_next (long *istart, long *iend)
{
  bool ret;

#ifdef HAVE_SYNC_BUILTINS
  ret = gomp_iter_guided_next (istart, iend);
#else
  struct gomp_thread *thr = gomp_thread ();
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_guided_next_locked (istart, iend);
  gomp_mutex_unlock (&thr->ts.work_share->lock);
#endif

  return ret;
}

bool
GOMP_loop_runtime_next (long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret = false;

#if defined(LIBGOMP_PROFILE_LOOP)
  union tick_t t0,t1;
  _GET_TICK(t0);
  if (PF(thr)->startcomp >0)
    PF(thr)->tcomp += t0.tick - PF(thr)->startcomp;
#endif
  
  switch (thr->ts.work_share->sched)
    {
    case GFS_STATIC:
    case GFS_AUTO:
      ret = gomp_loop_static_next (istart, iend);
      break;
    case GFS_DYNAMIC:
      ret = gomp_loop_dynamic_next (istart, iend);
      break;
    case GFS_GUIDED:
      ret = gomp_loop_guided_next (istart, iend);
      break;
#if defined(LIBGOMP_USE_ADAPTIVE)
    case GFS_ADAPTIVE:
      ret = gomp_iter_adaptive_next (istart, iend);
      break;
#endif
    default:
      abort ();
    }

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tpop += t1.tick - t0.tick;
  if (!ret)
    PF(thr)->startcomp = 0;
  else
    PF(thr)->startcomp = t1.tick;
  PF(thr)->iterlength += *iend-*istart;
#endif

  //printf("Thread: %i get [%li, %li)\n",thr->ts.team_id, *istart, *iend);
  return ret;
}

/* The *_ordered_*_next routines are called when the thread completes
   processing of the iteration block currently assigned to it.

   Returns true if there is work remaining to be performed; *ISTART and
   *IEND are filled with a new iteration block.  Returns false if all work
   has been assigned.  */

static bool
gomp_loop_ordered_static_next (long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  int test;

  gomp_ordered_sync ();
  gomp_mutex_lock (&thr->ts.work_share->lock);
  test = gomp_iter_static_next (istart, iend);
  if (test >= 0)
    gomp_ordered_static_next ();
  gomp_mutex_unlock (&thr->ts.work_share->lock);

  return test == 0;
}

static bool
gomp_loop_ordered_dynamic_next (long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  gomp_ordered_sync ();
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_dynamic_next_locked (istart, iend);
  if (ret)
    gomp_ordered_next ();
  else
    gomp_ordered_last ();
  gomp_mutex_unlock (&thr->ts.work_share->lock);

  return ret;
}

static bool
gomp_loop_ordered_guided_next (long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  gomp_ordered_sync ();
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_guided_next_locked (istart, iend);
  if (ret)
    gomp_ordered_next ();
  else
    gomp_ordered_last ();
  gomp_mutex_unlock (&thr->ts.work_share->lock);

  return ret;
}

bool
GOMP_loop_ordered_runtime_next (long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  
  switch (thr->ts.work_share->sched)
    {
    case GFS_STATIC:
    case GFS_AUTO:
      return gomp_loop_ordered_static_next (istart, iend);
    case GFS_DYNAMIC:
      return gomp_loop_ordered_dynamic_next (istart, iend);
    case GFS_GUIDED:
      return gomp_loop_ordered_guided_next (istart, iend);
    default:
      abort ();
    }
}



/* The GOMP_parallel_loop_* routines pre-initialize a work-share construct
   to avoid one synchronization once we get into the loop.  */

static void
gomp_parallel_loop_start (void (*fn) (void *), void *data,
			  unsigned num_threads, long start, long end,
			  long incr, enum gomp_schedule_type sched,
			  long chunk_size)
{
  struct gomp_team *team;
  num_threads = gomp_resolve_num_threads (num_threads, 0);
  team = gomp_new_team (num_threads);
#if defined(LIBGOMP_PROFILE_LOOP)
  union tick_t t0,t1;
  _GET_TICK(t0);
#endif
#if defined(LIBGOMP_PROFILE_LOOP)
  team->work_shares[0].ws_index= __sync_fetch_and_add(&work_shared_index, 1);
#endif
  gomp_loop_init (&team->work_shares[0], start, end, incr, sched, chunk_size );
#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(gomp_thread())->tinitloop += t1.tick - t0.tick;
#endif
  gomp_team_start (fn, data, num_threads, team);
}

void
GOMP_parallel_loop_static_start (void (*fn) (void *), void *data,
				 unsigned num_threads, long start, long end,
				 long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_STATIC, chunk_size);
}

void
GOMP_parallel_loop_dynamic_start (void (*fn) (void *), void *data,
				  unsigned num_threads, long start, long end,
				  long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_DYNAMIC, chunk_size);
}

void
GOMP_parallel_loop_guided_start (void (*fn) (void *), void *data,
				 unsigned num_threads, long start, long end,
				 long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_GUIDED, chunk_size);
}

void
GOMP_parallel_loop_runtime_start (void (*fn) (void *), void *data,
				  unsigned num_threads, long start, long end,
				  long incr)
{
  struct gomp_task_icv *icv = gomp_icv (false);
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    icv->run_sched_var, icv->run_sched_modifier);
}

/* The GOMP_loop_end* routines are called after the thread is told that
   all loop iterations are complete.  This first version synchronizes
   all threads; the nowait version does not.  */

void
GOMP_loop_end (void)
{
#if defined(LIBGOMP_PROFILE_LOOP)
  struct gomp_thread *thr = gomp_thread ();
  union tick_t t0,t1;
  _GET_TICK(t0);
  if (PF(thr)->startcomp >0)
    PF(thr)->tcomp += t0.tick - PF(thr)->startcomp;
#endif

  gomp_work_share_end ();

#if defined(LIBGOMP_USE_ADAPTIVE)
  /* Thierry: optimize case where adaptive loop was not used 
     Does the ICV is correct here ?
  */
  struct gomp_task_icv *icv = gomp_icv (false);
  if (icv->run_sched_var == GFS_ADAPTIVE)
    gomp_loop_adaptive_cleanup ();
#endif

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tendloop += t1.tick - t0.tick;
#endif
}

void
GOMP_loop_end_nowait (void)
{
#if defined(LIBGOMP_PROFILE_LOOP)
  struct gomp_thread *thr = gomp_thread ();
  union tick_t t0,t1;
  _GET_TICK(t0);
  if (PF(thr)->startcomp >0)
    PF(thr)->tcomp += t0.tick - PF(thr)->startcomp;
#endif

  gomp_work_share_end_nowait ();

#if defined(LIBGOMP_USE_ADAPTIVE)
  /* Thierry: optimize case where adaptive loop was not used
     Does the ICV is correct here ?
  */
  struct gomp_task_icv *icv = gomp_icv (false);
  if (icv->run_sched_var == GFS_ADAPTIVE)
  {
    gomp_loop_adaptive_cleanup ();
  }
#endif

#if defined(LIBGOMP_PROFILE_LOOP)
  _GET_TICK(t1);
  PF(thr)->tendloop += t1.tick - t0.tick;
#endif
}


/* We use static functions above so that we're sure that the "runtime"
   function can defer to the proper routine without interposition.  We
   export the static function with a strong alias when possible, or with
   a wrapper function otherwise.  */

#ifdef HAVE_ATTRIBUTE_ALIAS
extern __typeof(gomp_loop_static_start) GOMP_loop_static_start
	__attribute__((alias ("gomp_loop_static_start")));
extern __typeof(gomp_loop_dynamic_start) GOMP_loop_dynamic_start
	__attribute__((alias ("gomp_loop_dynamic_start")));
extern __typeof(gomp_loop_guided_start) GOMP_loop_guided_start
	__attribute__((alias ("gomp_loop_guided_start")));

extern __typeof(gomp_loop_ordered_static_start) GOMP_loop_ordered_static_start
	__attribute__((alias ("gomp_loop_ordered_static_start")));
extern __typeof(gomp_loop_ordered_dynamic_start) GOMP_loop_ordered_dynamic_start
	__attribute__((alias ("gomp_loop_ordered_dynamic_start")));
extern __typeof(gomp_loop_ordered_guided_start) GOMP_loop_ordered_guided_start
	__attribute__((alias ("gomp_loop_ordered_guided_start")));

extern __typeof(gomp_loop_static_next) GOMP_loop_static_next
	__attribute__((alias ("gomp_loop_static_next")));
extern __typeof(gomp_loop_dynamic_next) GOMP_loop_dynamic_next
	__attribute__((alias ("gomp_loop_dynamic_next")));
extern __typeof(gomp_loop_guided_next) GOMP_loop_guided_next
	__attribute__((alias ("gomp_loop_guided_next")));

extern __typeof(gomp_loop_ordered_static_next) GOMP_loop_ordered_static_next
	__attribute__((alias ("gomp_loop_ordered_static_next")));
extern __typeof(gomp_loop_ordered_dynamic_next) GOMP_loop_ordered_dynamic_next
	__attribute__((alias ("gomp_loop_ordered_dynamic_next")));
extern __typeof(gomp_loop_ordered_guided_next) GOMP_loop_ordered_guided_next
	__attribute__((alias ("gomp_loop_ordered_guided_next")));
#else
bool
GOMP_loop_static_start (long start, long end, long incr, long chunk_size,
			long *istart, long *iend)
{
  return gomp_loop_static_start (start, end, incr, chunk_size, istart, iend);
}

bool
GOMP_loop_dynamic_start (long start, long end, long incr, long chunk_size,
			 long *istart, long *iend)
{
  return gomp_loop_dynamic_start (start, end, incr, chunk_size, istart, iend);
}

bool
GOMP_loop_guided_start (long start, long end, long incr, long chunk_size,
			long *istart, long *iend)
{
  return gomp_loop_guided_start (start, end, incr, chunk_size, istart, iend);
}

bool
GOMP_loop_ordered_static_start (long start, long end, long incr,
				long chunk_size, long *istart, long *iend)
{
  return gomp_loop_ordered_static_start (start, end, incr, chunk_size,
					 istart, iend);
}

bool
GOMP_loop_ordered_dynamic_start (long start, long end, long incr,
				 long chunk_size, long *istart, long *iend)
{
  return gomp_loop_ordered_dynamic_start (start, end, incr, chunk_size,
					  istart, iend);
}

bool
GOMP_loop_ordered_guided_start (long start, long end, long incr,
				long chunk_size, long *istart, long *iend)
{
  return gomp_loop_ordered_guided_start (start, end, incr, chunk_size,
					 istart, iend);
}

bool
GOMP_loop_static_next (long *istart, long *iend)
{
  return gomp_loop_static_next (istart, iend);
}

bool
GOMP_loop_dynamic_next (long *istart, long *iend)
{
  return gomp_loop_dynamic_next (istart, iend);
}

bool
GOMP_loop_guided_next (long *istart, long *iend)
{
  return gomp_loop_guided_next (istart, iend);
}

bool
GOMP_loop_ordered_static_next (long *istart, long *iend)
{
  return gomp_loop_ordered_static_next (istart, iend);
}

bool
GOMP_loop_ordered_dynamic_next (long *istart, long *iend)
{
  return gomp_loop_ordered_dynamic_next (istart, iend);
}

bool
GOMP_loop_ordered_guided_next (long *istart, long *iend)
{
  return gomp_loop_ordered_guided_next (istart, iend);
}
#endif

#include <stdio.h>

void libgomp_iter_next (void) 
{
#if 0
  struct gomp_thread *thr = gomp_thread ();

  if (thr->current_chunk != NULL) {
    printf ("[libgomp] Thread %i computing iteration %li from range [%li,%li]\n", 
	    thr->ts.team_id, 
	    thr->current_chunk->current, 
	    thr->current_chunk->start, 
	    thr->current_chunk->end);
    thr->current_chunk->current++; 
  }

  if (thr->current_chunk->current == 5) {
    thr->current_chunk->end = 50;
    thr->ts.work_share->end = 50;
    thr->ts.work_share->next = 50;
    *(thr->current_chunk->pend) = 50;
    thr->ts.team->work_shares[thr->ts.team_id].end = 50;
  }
#endif
}
