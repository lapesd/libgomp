/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
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

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "libgomp.h"

/*============================================================================*
 * Workload Information                                                       *
 *============================================================================*/

#define NR_LOOPS 32

/**
 * @brief Tasks.
 */
unsigned *__tasks;

/**
 * @brief Number of tasks.
 */
unsigned __ntasks;

struct loop
{
  char *name;
  unsigned *taskmap;
  bool override;
};

static struct loop loops[NR_LOOPS] = { {NULL, NULL, false} };
static int curr_loop = -1;

unsigned __nchunks = 1;

static void init_loop_struct(struct loop *loop,
                             const char *name) {
  size_t name_len = strlen(name) + 1;
  loop->name = calloc(name_len, 1);
  strncpy(loop->name, name, name_len);
}

/**
 * @brief Register the next parallel loop to the runtime system.
 *
 * @param loop_name The name referring to this loop.
 *
 * @return Returns a unique ID for the loop.
 */
unsigned omp_loop_register(const char *loop_name)
{
  for (size_t i = 0; i < NR_LOOPS; i++) {
    if (loops[i].name == NULL) {
      init_loop_struct(&loops[i], loop_name);
      return i;
    }
  }

  fprintf(stderr, "[binlpt] Too many loops, aborting.\n");
  abort();

  return 0;
}

/**
 * @brief Sets the workload of the next parallel for loop.
 *
 * @param loop_id     The ID of the loop to attach workload information to.
 * @param tasks       Load of iterations.
 * @param ntasks      Number of tasks.
 * @param override    Boolean flag to decide whether we should compute the
 *                    task mapping again or use the preexisting one.
 */
void omp_set_workload(unsigned loop_id,
                      unsigned *tasks,
                      unsigned ntasks,
                      bool override)
{
  assert((0 <= loop_id) && (loop_id < NR_LOOPS));

  loops[loop_id].override = override;
  curr_loop = loop_id;

	__tasks = tasks;
	__ntasks = ntasks;
}

/*============================================================================*
 * Workload Sorting                                                           *
 *============================================================================*/

#define N 128

/*
 * Exchange two numbers.
 */
#define exch(a, b, t) \
	{ (t) = (a); (a) = (b); (b) = (t); }

/*
 * Insertion sort.
 */
static void insertion(unsigned *map, unsigned *a, unsigned n)
{
	unsigned t;    /* Temporary value. */
	unsigned i, j; /* Loop indexes.    */
	
	/* Sort. */
	for (i = 0; i < (n - 1); i++)
	{
		for (j = i + 1; j < n; j++)
		{
			/* Swap. */
			if (a[j] < a[i])
			{
				exch(a[i], a[j], t);
				exch(map[i], map[j], t);
			}
		}
	}
}

/*
 * Quicksort algorithm.
 */
void quicksort(unsigned *map, unsigned *a, unsigned n)
{
	unsigned i, j;
	unsigned p, t;
    
    /* End recursion. */
	if (n < N)
	{
		insertion(map, a, n);
		return;
	}
    
	/* Pivot stuff. */
	p = a[n/2];
	for (i = 0, j = n - 1; /* noop */ ; i++, j--)
	{
		while (a[i] < p)
			i++;
		while (p < a[j])
			j--;
		if (i >= j)
			break;
		exch(a[i], a[j], t);
		exch(map[i], map[j], t);
	}
    
	quicksort(map, a, i);
	quicksort(map, a + i, n - i);
}
 
/*
 * Sorts an array of numbers.
 */
void sort(unsigned *a, unsigned n, unsigned *map)
{
	unsigned i;	

	/* Create map. */
	for (i = 0; i < n; i++)
		map[i] = i;

	insertion(map, a, n);
} 

/*============================================================================*
 * SRR Loop Scheduler                                                         *
 *============================================================================*/

/**
 * @brief Smart Round-Robin loop scheduler.
 *
 * @param tasks    Target tasks.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 *
 * @returns Iteration scheduling map.
 */
static unsigned *srr_balance(unsigned *tasks, unsigned ntasks, unsigned nthreads)
{
	unsigned k;               /* Scheduling offset. */
	unsigned tid;             /* Current thread ID. */
	unsigned i, j;            /* Loop indexes.      */
	unsigned *taskmap;        /* Task map.          */
	unsigned sortmap[ntasks]; /* Sorting map.       */
	unsigned load[ntasks];    /* Assigned load.     */

	/* Initialize scheduler data. */
	taskmap = malloc(ntasks*sizeof(unsigned));
	assert(taskmap != NULL);
	memset(load, 0, ntasks * sizeof(unsigned));

	/* Sort tasks. */
	sort(tasks, ntasks, sortmap);

	/* Assign tasks to threads. */
	tid = 0;
	k = ntasks & 1;

	for (i = k; i < k + (ntasks - k); i++)
	{
		unsigned l = sortmap[i];
		unsigned r = sortmap[ntasks - ((i - k) + 1)];

		taskmap[l] = tid;
		taskmap[r] = tid;

		load[tid] += tasks[l] + tasks[r];

		/* Wrap around. */
		tid = (tid + 1)%nthreads;
	}

	/* Assign remaining tasks. */
	for (i = k; i > 0; i--)
	{
		unsigned leastoverload;

		/* Find least overload thread. */
		leastoverload = 0;
		for (j = 1; j < nthreads; j++)
		{
			if (load[j] < load[leastoverload])
				leastoverload = j;
		}

		taskmap[sortmap[i - 1]] = leastoverload;

		load[leastoverload] += tasks[sortmap[i - 1]];
	}

	return (taskmap);
}

/*============================================================================*
 * BIN+LPT Loop Scheduler                                                     *
 *============================================================================*/

/**
 * @brief Computes the cummulative sum of an array.
 *
 * @param a Target array.
 * @param n Size of target array.
 *
 * @returns Commulative sum.
 */
static unsigned *compute_cummulativesum(const unsigned *a, unsigned n)
{
	unsigned i;
	unsigned *sum;

	sum = malloc(n*sizeof(unsigned));
	assert(sum != NULL);

	for (sum[0] = 0, i = 1; i < n; i++)
		sum[i] = sum[i - 1] + a[i - 1];

	return (sum);
}

/**
 * @brief Computes chunk sizes.
 *
 * @param tasks   Target tasks.
 * @param ntasks  Number of tasks.
 * @param nchunks Number of chunks.
 *
 * @returns Chunk sizes.
 */
static unsigned *compute_chunksizes(const unsigned *tasks, unsigned ntasks, unsigned nchunks)
{
	unsigned i, k;
	unsigned chunkweight;
	unsigned *chunksizes, *workload;

	chunksizes = calloc(nchunks, sizeof(unsigned));
	assert(chunksizes != NULL);
	
	workload = compute_cummulativesum(tasks, ntasks);

	chunkweight = (workload[ntasks - 1] + tasks[ntasks - 1])/nchunks;

	/* Compute chunksizes. */
	for (k = 0, i = 0; i < ntasks; /* noop */)
	{
		unsigned j = ntasks;

		if (k < (nchunks - 1))
		{
			for (j = i + 1; j < ntasks; j++)
			{
				if (workload[j] - workload[i] > chunkweight)
					break;
			}
		}

		chunksizes[k] = j - i;
		i = j;
		k++;
	}

	/* House keeping. */
	free(workload);

	return (chunksizes);
}

/**
 * @brief Computes chunks.
 */
static unsigned *compute_chunks(const unsigned *tasks, unsigned ntasks, const unsigned *chunksizes, unsigned nchunks)
{
	unsigned i, k;    /* Loop indexes. */
	unsigned *chunks; /* Chunks.       */

	chunks = calloc(nchunks, sizeof(unsigned));
	assert(chunks != NULL);

	/* Compute chunks. */
	for (i = 0, k = 0; i < nchunks; i++)
	{
		unsigned j;

		assert(k <= ntasks);

		for (j = 0; j < chunksizes[i]; j++)
			chunks[i] += tasks[k++];
	}

	return (chunks);
}

/**
 * @brief Bin Packing Longest Processing Time First loop scheduler.
 */
static unsigned *binlpt_balance(unsigned *tasks, unsigned ntasks, unsigned nthreads)
{
	unsigned i;               /* Loop index.       */
	unsigned *taskmap;        /* Task map.         */
	unsigned sortmap[ntasks]; /* Sorting map.       */
	unsigned *load;           /* Assigned load.    */
	unsigned *chunksizes;     /* Chunks sizes.     */
	unsigned *chunks;         /* Chunks.           */
	unsigned *chunkoff;       /* Offset to chunks. */

  //printf("[binlpt] Balancing loop %s:%i\n", loops[curr_loop].filename, loops[curr_loop].line);

	/* Initialize scheduler data. */
	taskmap = calloc(ntasks, sizeof(unsigned));
	assert(taskmap != NULL);
	load = calloc(nthreads, sizeof(unsigned));
	assert(load != NULL);

	chunksizes = compute_chunksizes(tasks, ntasks, __nchunks);
	chunks = compute_chunks(tasks, ntasks, chunksizes, __nchunks);
	chunkoff = compute_cummulativesum(chunksizes, __nchunks);

	/* Sort tasks. */
	sort(chunks, __nchunks, sortmap);
	
	for (i = __nchunks; i > 0; i--)
	{
		unsigned j;
		unsigned tid;

		if (chunks[i - 1] == 0)
			continue;

		tid = 0;
		for (j = 1; j < nthreads; j++)
		{
			if (load[j] < load[tid])
				tid = j;
		}

		for (j = 0; j < chunksizes[sortmap[i - 1]]; j++)
			taskmap[chunkoff[sortmap[i - 1]] + j] = tid;
		load[tid] += chunks[i - 1];
	}

	/* House keeping. */
	free(chunkoff);
	free(chunks);
	free(chunksizes);
	free(load);
	
	return (taskmap);
}

/*============================================================================*
 * Hacked LibGomp Routines                                                    *
 *============================================================================*/

/* Initialize the given work share construct from the given arguments.  */

static inline void
gomp_loop_init (struct gomp_work_share *ws, long start, long end, long incr,
		enum gomp_schedule_type sched, long chunk_size, unsigned num_threads)
{
  ws->sched = sched;
  ws->chunk_size = chunk_size;
  /* Canonicalize loops that have zero iterations to ->next == ->end.  */
  ws->end = ((incr > 0 && start > end) || (incr < 0 && start < end))
    ? start : end;
  ws->incr = incr;
  ws->next = start;


  switch (sched) {
  case GFS_DYNAMIC:
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
    break;

  case GFS_BINLPT:
    {
      if (num_threads == 0)
        {
          struct gomp_thread *thr = gomp_thread ();
          struct gomp_team *team = thr->ts.team;
          num_threads = (team != NULL) ? team->nthreads : 1;
        }

      __nchunks = chunk_size;
      if (__nchunks == 1)
        __nchunks = num_threads;
    }
  case GFS_SRR:
    {
      unsigned *(*balance)(unsigned *, unsigned, unsigned);
      balance = (sched == GFS_SRR) ? srr_balance : binlpt_balance;
      if (num_threads == 0)
        {
          struct gomp_thread *thr = gomp_thread ();
          struct gomp_team *team = thr->ts.team;
          num_threads = (team != NULL) ? team->nthreads : 1;
        }

      struct loop *loop = &loops[curr_loop];
      if (loop->override || loop->taskmap == NULL) {
        /* Refresh the mapping. */
        if (loop->taskmap != NULL) {
          free(loop->taskmap);
        }
        loop->taskmap = balance(__tasks, __ntasks, num_threads);
      }
      ws->taskmap = loop->taskmap;

      ws->loop_start = start;

      ws->thread_start = (unsigned *) calloc(num_threads, sizeof(int));
    }
    break;

  default:
    break;
  }
}

static bool
gomp_loop_static_start (long start, long end, long incr, long chunk_size,
			long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();

  thr->ts.static_trip = 0;
  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_STATIC, chunk_size, 0);
      gomp_work_share_init_done ();
    }

  return !gomp_iter_static_next (istart, iend);
}

static bool
gomp_loop_dynamic_start (long start, long end, long incr, long chunk_size,
			 long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_DYNAMIC, chunk_size, 0);
      gomp_work_share_init_done ();
    }

#ifdef HAVE_SYNC_BUILTINS
  ret = gomp_iter_dynamic_next (istart, iend);
#else
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_dynamic_next_locked (istart, iend);
  gomp_mutex_unlock (&thr->ts.work_share->lock);
#endif

  return ret;
}

static bool
gomp_loop_guided_start (long start, long end, long incr, long chunk_size,
			long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_GUIDED, chunk_size, 0);
      gomp_work_share_init_done ();
    }

#ifdef HAVE_SYNC_BUILTINS
  ret = gomp_iter_guided_next (istart, iend);
#else
  gomp_mutex_lock (&thr->ts.work_share->lock);
  ret = gomp_iter_guided_next_locked (istart, iend);
  gomp_mutex_unlock (&thr->ts.work_share->lock);
#endif

  return ret;
}

static bool
gomp_loop_binlpt_start (long start, long end, long incr, long chunk_size,
		       long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_BINLPT, chunk_size, 0);
      gomp_work_share_init_done ();
    }

  ret = gomp_iter_binlpt_next (istart, iend);

  return ret;
}

static bool
gomp_loop_srr_start (long start, long end, long incr, long chunk_size,
		       long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();
  bool ret;

  if (gomp_work_share_start (false))
    {
      gomp_loop_init (thr->ts.work_share, start, end, incr,
		      GFS_SRR, chunk_size, 0);
      gomp_work_share_init_done ();
    }

  ret = gomp_iter_srr_next (istart, iend);

  return ret;
}

bool
GOMP_loop_runtime_start (long start, long end, long incr,
			 long *istart, long *iend)
{
  struct gomp_task_icv *icv = gomp_icv (false);
  switch (icv->run_sched_var)
    {
    case GFS_STATIC:
      return gomp_loop_static_start (start, end, incr, icv->run_sched_modifier,
				     istart, iend);
    case GFS_DYNAMIC:
      return gomp_loop_dynamic_start (start, end, incr, icv->run_sched_modifier,
				      istart, iend);
    case GFS_GUIDED:
      return gomp_loop_guided_start (start, end, incr, icv->run_sched_modifier,
				     istart, iend);

    case GFS_BINLPT:
      return gomp_loop_binlpt_start (start, end, incr, icv->run_sched_modifier, istart, iend);
    case GFS_SRR:
      return gomp_loop_srr_start (start, end, incr, icv->run_sched_modifier, istart, iend);

    case GFS_AUTO:
      /* For now map to schedule(static), later on we could play with feedback
	 driven choice.  */
      return gomp_loop_static_start (start, end, incr, 0, istart, iend);
    default:
      abort ();
    }
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
		      GFS_STATIC, chunk_size, 0);
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
		      GFS_DYNAMIC, chunk_size, 0);
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
		      GFS_GUIDED, chunk_size, 0);
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

static bool
gomp_loop_binlpt_next (long *istart, long *iend)
{
  return gomp_iter_binlpt_next (istart, iend);
}

static bool
gomp_loop_srr_next (long *istart, long *iend)
{
  return gomp_iter_srr_next (istart, iend);
}

bool
GOMP_loop_runtime_next (long *istart, long *iend)
{
  struct gomp_thread *thr = gomp_thread ();

  switch (thr->ts.work_share->sched)
    {
    case GFS_STATIC:
    case GFS_AUTO:
      return gomp_loop_static_next (istart, iend);
    case GFS_DYNAMIC:
      return gomp_loop_dynamic_next (istart, iend);
    case GFS_GUIDED:
      return gomp_loop_guided_next (istart, iend);
    case GFS_BINLPT:
      return gomp_loop_binlpt_next (istart, iend);
    case GFS_SRR:
      return gomp_loop_srr_next (istart, iend);
    default:
      abort ();
    }
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
			  long chunk_size, unsigned int flags)
{
  struct gomp_team *team;

  num_threads = gomp_resolve_num_threads (num_threads, 0);
  team = gomp_new_team (num_threads);
  gomp_loop_init (&team->work_shares[0], start, end, incr, sched, chunk_size, num_threads);
  gomp_team_start (fn, data, num_threads, flags, team);
}

void
GOMP_parallel_loop_static_start (void (*fn) (void *), void *data,
				 unsigned num_threads, long start, long end,
				 long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_STATIC, chunk_size, 0);
}

void
GOMP_parallel_loop_dynamic_start (void (*fn) (void *), void *data,
				  unsigned num_threads, long start, long end,
				  long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_DYNAMIC, chunk_size, 0);
}

void
GOMP_parallel_loop_guided_start (void (*fn) (void *), void *data,
				 unsigned num_threads, long start, long end,
				 long incr, long chunk_size)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_GUIDED, chunk_size, 0);
}

void
GOMP_parallel_loop_runtime_start (void (*fn) (void *), void *data,
				  unsigned num_threads, long start, long end,
				  long incr)
{
  struct gomp_task_icv *icv = gomp_icv (false);
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    icv->run_sched_var, icv->run_sched_modifier, 0);
}

ialias_redirect (GOMP_parallel_end)

void
GOMP_parallel_loop_static (void (*fn) (void *), void *data,
			   unsigned num_threads, long start, long end,
			   long incr, long chunk_size, unsigned flags)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_STATIC, chunk_size, flags);
  fn (data);
  GOMP_parallel_end ();
}

void
GOMP_parallel_loop_dynamic (void (*fn) (void *), void *data,
			    unsigned num_threads, long start, long end,
			    long incr, long chunk_size, unsigned flags)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_DYNAMIC, chunk_size, flags);
  fn (data);
  GOMP_parallel_end ();
}

void
GOMP_parallel_loop_guided (void (*fn) (void *), void *data,
			  unsigned num_threads, long start, long end,
			  long incr, long chunk_size, unsigned flags)
{
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    GFS_GUIDED, chunk_size, flags);
  fn (data);
  GOMP_parallel_end ();
}

void
GOMP_parallel_loop_runtime (void (*fn) (void *), void *data,
			    unsigned num_threads, long start, long end,
			    long incr, unsigned flags)
{
  struct gomp_task_icv *icv = gomp_icv (false);
  gomp_parallel_loop_start (fn, data, num_threads, start, end, incr,
			    icv->run_sched_var, icv->run_sched_modifier,
			    flags);
  fn (data);
  GOMP_parallel_end ();
}

/* The GOMP_loop_end* routines are called after the thread is told that
   all loop iterations are complete.  The first two versions synchronize
   all threads; the nowait version does not.  */

void
GOMP_loop_end (void)
{
  gomp_work_share_end ();
}


bool
GOMP_loop_end_cancel (void)
{
  return gomp_work_share_end_cancel ();
}

void
GOMP_loop_end_nowait (void)
{
  gomp_work_share_end_nowait ();
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
