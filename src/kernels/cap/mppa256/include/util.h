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
	 * Sends data.
	 */
	extern uint64_t data_send(int outfd, void *data, size_t n);
	
	/*
	 * Receives data.
	 */
	extern uint64_t data_receive(int infd, void *data, size_t n);

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
	 * Prints a warning message.
	 */
	extern void warning(const char *msg);

#endif /* UTIL_H_ */
