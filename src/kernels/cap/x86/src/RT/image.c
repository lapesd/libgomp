/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <assert.h>
#include <stdio.h>
#include <util.h>
#include "image.h"

/*
 * Creates a image.
 */
struct image *image_create(unsigned width, unsigned height)
{
	struct image *img;  /* Image.       */
	unsigned dimension; /* width*height */
	
	/* Sanity check. */
	assert(width > 0);
	assert(height > 0);
	
	img = smalloc(sizeof(struct image));
	
	/* Initialize image. */
	dimension = width*height;
	img->width = width;
	img->height = height;
	img->dimension = dimension;
	img->pixels = smalloc(dimension*sizeof(struct pixel));
	
	return (img);
}
	
/*
 * Destroys a image.
 */
extern void image_destroy(struct image *img)
{
	/* Sanity check. */
	assert(img != NULL);
	
	free(img->pixels);
	free(img);
}

/*
 * Exports an image to PPM format.
 */
static void image_export_ppm(FILE *file, struct image *img)
{
	unsigned i;
	
	fprintf(file, "P6\n%u %u\n255\n", img->width, img->height);
	for (i = 0; i < img->dimension; i++) {
		fprintf(file, "%c%c%c", 
			img->pixels[i].r,
			img->pixels[i].g,
			img->pixels[i].b);
	}
}

/*
 * Exports an image to a file type.
 */
extern void image_export(const char *filename, struct image *img, unsigned type)
{
	FILE *file;
	
	/* Sanity check. */
	assert(filename != NULL);
	assert(img != NULL);
	
	/* Open output file. */
	file = fopen(filename, "w");
	if (file == NULL) {
		error("cannot image_export()");
	}
	
	/* Parse file type. */
	switch (type)
	{
		/* Portable Pixmap Image. */
		case IMAGE_PPM:
			image_export_ppm(file, img);
			break;
		
		default:
			warning("unknown image file type");
	}
	
	/* Close output file. */
	fclose(file);
}
