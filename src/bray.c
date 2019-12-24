/*
 * Brian Chrzanowski
 * Tue Dec 24, 2019 17:28
 *
 * Brian's Raytracer
 *
 * NOTE
 * At this time, I'm generally interested in working on a raytracer that
 * really only operates off of triangles. This means that the only primitive
 * I'd have, is a triangle.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH  (1024)
#define HEIGHT (768)

struct pixel_t {
	u8 r, g, b;
};
typedef struct pixel_t pixel;

int main(int argc, char **argv)
{
	pixel *img;
	s32 w, h;
	int rc;

	w = WIDTH;
	h = HEIGHT;

	img = calloc(w * h, sizeof(struct pixel_t));

	rc = stbi_write_png("output.png", w, h, 3, img, 0);
	if (!rc) {
		fprintf(stderr, "Error, stbi_write_png failed\n");
		exit(1);
	}

	free(img);

	return 0;
}

