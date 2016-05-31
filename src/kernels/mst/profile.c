/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <stdio.h>

#include <papi.h>

#include <util.h>

/**
 * @brief Number of events to profile.
 */
#define NR_EVENTS 4

/**
 * @brief Events to profile.
 */
static int events[NR_EVENTS] = {
	PAPI_L1_DCM, /* L1 data cache misses.   */
	PAPI_L2_DCM, /* L2 data cache misses.   */
	PAPI_L2_DCA, /* L2 data cache accesses. */
	PAPI_L3_DCA  /* L3 daca cache accesses. */
};

/**
 * @brief Hardware counters.
 */
static long long hwcounters[NR_EVENTS];

/**
 * @brief Start profiling.
 */
void profile_start(void)
{
	if (PAPI_start_counters(events, NR_EVENTS) != PAPI_OK)
		error("failed to setup PAPI");
}

/**
 * @brief End profiling.
 */
void profile_end(void)
{
	if (PAPI_stop_counters(hwcounters, sizeof(events)) != PAPI_OK)
		error("failed to read hardware counters");
		
}

/**
 * @brief Dump profiling statistiscs.
 */
void profile_dump(void)
{
	fprintf(stderr, "L1 Misses: %lld\n", hwcounters[0]);
	fprintf(stderr, "L2 Misses: %lld\n", hwcounters[1]);
	fprintf(stderr, "L2 Accesses: %lld\n", hwcounters[2]);
	fprintf(stderr, "L3 Accesses: %lld\n", hwcounters[3]);
}
