/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * timer.c - Timer library implementation.
 */

#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>

/* Timer residual error. */
static uint64_t timer_error = 0;

/*
 * Gets the current timer value.
 */
uint64_t timer_get(void)
{
	uint64_t ret;
	struct timeval t;

	gettimeofday(&t, NULL);
	ret = 1000000 * ((uint64_t) t.tv_sec);
	ret += (uint64_t) t.tv_usec;

	return ret;
}

/*
 * Computers the difference between two timers.
 */
uint64_t timer_diff(uint64_t t1, uint64_t t2)
{
	return (t2 - t1 - timer_error);
}

/*
 * Initializes the timer.
 */
void timer_init(void)
{
  uint64_t start, end;
  
  start = timer_get();
  end = timer_get();
  
  timer_error = (end - start);
}
