/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * power.c - Power utility library for Intel Xeon Phi
 */

#include <assert.h>
#include <stdio.h>
#include <pthread.h>

/*
 * Power information buffer. 
 */
struct
{
	unsigned tot0; /* Total power time window 0 (uW).                      */
	unsigned tot1; /* Total power time window 1 (uW).                      */
	unsigned pcie; /* Power measured at the PCI-express input.             */
	unsigned inst; /* Instantaneous power consumption reading (uW).        */
	unsigned imax; /* Maximum instantaneous power consumption observed.    */
	unsigned c2x3; /* Power measured at the 2x3 connector.                 */
	unsigned c2x4; /* Power measured at the 2x4 connector.                 */
	unsigned vccp; /* Power supply to the cores.                           */
	unsigned vddg; /* Power supply to everything but the cores and memory. */
	unsigned vddq; /* Power supply to the memory subsystem.                */
} power_buffer;

/*
 * /sys/class/micras/power
 */
static const char *micras_power_file = "/sys/class/micras/power";
static FILE *micras_power = NULL;


static const long time_stamp = 50000000L;
static pthread_t tid;
static volatile double avg = 0;
static unsigned count = 0;
static unsigned live;

/*
 * Gets power information.
 */
static double power_get(void)
{
	int error;
	
	micras_power = fopen(micras_power_file, "r");
	assert(micras_power != NULL);
	
	error = fscanf(micras_power, "%u", &power_buffer.tot0);
	((void)error);
	
	fclose(micras_power);
	
	return (power_buffer.tot0);
}

/*
 * Listen to micras power file.
 */
static void *power_listen(void *unused)
{
	struct timespec ts;
	
	((void)unused);
	
	ts.tv_sec = 0;
	ts.tv_nsec = time_stamp;
	
	do
	{
		avg += power_get();
		count++;
	
		nanosleep(&ts, NULL);
	} while (live);
	
	avg /= count;
	
	return (NULL);
}

/*
 * Initializes power measurement utility.
 */
void power_init(void)
{
	live = 1;
	
	pthread_create(&tid, NULL, power_listen, NULL);
}

/*
 * Terminates power measurement utility.
 */
double power_end(void)
{
	live = 0;
	pthread_join(tid, NULL);
	
	return (avg);
}
