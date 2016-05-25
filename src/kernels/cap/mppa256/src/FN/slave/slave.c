/* 
 * File:   master.c - Mutually Friendly Numbers master process.
 * Authors: 
 *		Matheus A. Souza <matheusalcantarasouza@gmail.com>
 *      Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * Copyright(C) 2014
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <mppaipc.h>
#include <omp.h>
#include <stdint.h>
#include <stdlib.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>

typedef struct {
    long int number;
    long int numerator;
    long int denominator;
} Item;

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

static Item task[65536];

static int tasksize;

static void syncNumbers(void)
{
    data_send(outfd, &task, tasksize*sizeof(Item));
}

static void getwork(void)
{
	data_receive(infd, &tasksize, sizeof(int));
	data_receive(infd, &task, tasksize*sizeof(Item));
}

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
  int c;
  
  /* Compute greatest common divisor. */
  while (a != 0)
  {
     c = a;
     a = b%a;
     b = c;
  }
  
  return (b);
}

/*
 * Some of divisors.
 */
static int sumdiv(int n)
{
	int sum;    /* Sum of divisors. */
	int factor; /* Working factor.  */
	
	sum = 1 + n;
	
	/* Compute sum of divisors. */
	for (factor = 2; factor < n; factor++)
	{
		/* Divisor found. */
		if ((n%factor) == 0)
			sum += factor;
	}
	
	return (sum);
}

/*
 * Computes friendly numbers.
 */
void friendly_numbers(void) 
{
	int n; /* Divisor.      */
	int i; /* Loop indexes. */
	
	/* Compute abundances. */
	#pragma omp parallel for private(i, n) default(shared)
	for (i = 0; i < tasksize; i++) 
	{		
		task[i].numerator = sumdiv(task[i].number);
		task[i].denominator = i;
				
		n = gcd(task[i].numerator, task[i].denominator);
		task[i].numerator /= n;
		task[i].denominator /= n;
	}
}


int main(int argc, char **argv)
{
	timer_init();

    ((void) argc);
    
    total = 0;
    
    rank = atoi(argv[0]);
    
    open_noc_connectors();
	
    getwork();

	start = timer_get();
    friendly_numbers();
	end = timer_get();

    syncNumbers();

	total = timer_diff(start, end);
	data_send(outfd, &total, sizeof(uint64_t));

    /* Close channels. */
    mppa_close(infd);
    mppa_close(outfd);

    mppa_exit(0);
    return (0);
}
