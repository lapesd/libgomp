/*
 * Copyright(C) 2014 	Alyson Deives Pereira <alyson.deives@outlook.com>,
 * 			Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * fast.h - FAST Corner Detection  kernel library.
 */

#ifndef _FAST_H_
#define _FAST_H_

	/*
	 * Maximum thread num.
	 */
	#define MAX_THREADS (256)
	
	/*
	 * Threshold value between central pixel and neighboor pixel.
	 */
	#define THRESHOLD (20)
	
	/*
	 * Mask radius.
	 */
	#define MASK_RADIUS (3)

	/*
	 * FAST corner detection.
	 */
	extern int
	fast(char *img, int imgsize, int *mask);

#endif /* _FAST_H_ */
