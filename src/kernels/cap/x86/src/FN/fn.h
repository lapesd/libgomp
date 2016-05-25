/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * fh.h - Friendly Numbers kernel library.
 */

#ifndef _FN_H_
#define _FN_H_

	/*
	 * Balances workload.
	 */
	extern void balance(int *work, int n, int k);

	/*
	 * Computes friendly numbers.
	 */
	extern int friendly_numbers(int start, int end);

#endif /* _FN_H_ */
