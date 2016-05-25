/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * gf.h - Gaussian Filter kernel library.
 */

#ifndef _GF_H_
#define _GF_H_

	/*
	 * pi
	 */
	#define PI 3.14159265359
	
	/*
	 * e
	 */
	#define E 2.71828182845904
	
	/*
	 * Standard deviation.
	 */
	#define SD 0.8

	/*
	 * Gaussian filter.
	 */
	extern void 
	gauss_filter(unsigned char *img, int imgsize, double *mask, int masksize);

#endif /* _GF_H_ */
