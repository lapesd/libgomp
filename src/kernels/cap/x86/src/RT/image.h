/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#ifndef IMAGE_H_
#define IMAGE_H_

	/*
	 * Pixel.
	 */
	struct pixel
	{
		unsigned char r; /* Red value.   */
		unsigned char g; /* Green value. */
		unsigned char b; /* Blue value.  */
	};

	/* Image file types. */
	#define IMAGE_PPM 0 /* ppm file type. */

	/*
	 * Raw image.
	 */
	struct image
	{
		unsigned width;       /* Width (in pixels).   */
		unsigned height;      /* Height (int pixels). */
		unsigned dimension;   /* width*height.        */
		struct pixel *pixels; /* Pixels.              */
	};
	
	/*
	 * Opaque pointer to a image.
	 */
	typedef struct image * image_t;
	
	#define IMAGE(img, x, y) \
		(img->pixels[(y)*img->width + (x)])
	
	/*
	 * Creates an image.
	 */
	extern image_t image_create(unsigned width, unsigned height);
	
	/*
	 * Destroys an image.
	 */
	extern void image_destroy(image_t img);
	
	/*
	 * Exports an image to a file type.
	 */
	extern void image_export(const char *filename, image_t img, unsigned type);

#endif /* IMAGE_H_ */
