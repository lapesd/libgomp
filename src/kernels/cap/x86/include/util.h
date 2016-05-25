/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * <util.h> - Utility library.
 */

#ifndef UTIL_H_
#define UTIL_H_
	
	#include <stdint.h>
	#include <stdlib.h>

	/*
	 * Prints an error message and exits.
	 */
	extern void error(const char *msg);
	
	/*
	 * Generates a normal number.
	 */
	extern double normalnum(double mu, double sigma);
	
	/*
	 * Generates a random number.
	 */
	extern unsigned randnum(void);
	
	/*
	 * Safe calloc().
	 */
	extern void *scalloc(size_t nmemb, size_t size);
	
	/*
	 * Safe malloc().
	 */
	extern void *smalloc(size_t size);
	
	/*
	 * Initializes the random number generator.
	 */
	extern void srandnum(int seed);
	
	/*
	 * Safe realloc().
	 */
	extern void *srealloc(void *p, size_t size);
	
	/*
	 * Prints a warning message.
	 */
	extern void warning(const char *msg);
	
	/*
	 * Initializes power measurement utility.
	 */
	extern void power_init(void);
	
	/*
	 * Terminates power measurement utility.
	 */
	extern double power_end(void);

#endif /* UTIL_H_ */
