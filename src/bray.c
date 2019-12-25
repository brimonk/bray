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
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "common.h"
#include "math.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH  (1024)
#define HEIGHT (768)

struct material_t {
	vec3f color;
};

struct sphere_t {
	vec3f p;
	f32 r;
	u32 mat;
};

struct plane_t {
	vec3f n;
	f32 d;
	u32 mat;
};

struct world_t {
	struct material_t *materials;
	u32 materials_cnt;
	u32 materials_len;
	struct plane_t *planes;
	u32 planes_cnt;
	u32 planes_len;
	struct sphere_t *spheres;
	u32 spheres_cnt;
	u32 spheres_len;
};

/* R_Main : rendering main function */
int R_Main(vec3f *framebuffer, s32 w, s32 h);

/* R_RayCast : cast a ray into the world, returning the color vector */
vec3f R_RayCast(struct world_t *world, vec3f ray_origin, vec3f ray_direction);

/* R_WorldGen : generates or loads the world up */
struct world_t *R_WorldGen();

/* R_WorldFree : frees the world structure */
void R_WorldFree(struct world_t *world);

int main(int argc, char **argv)
{
	vec3f *framebuffer;
	vec3ub *img;
	s32 w, h;
	s32 i, j;
	s32 idx;
	int rc;

	w = WIDTH;
	h = HEIGHT;

	img = calloc(w * h, sizeof(*img));
	framebuffer = calloc(w * h, sizeof(*framebuffer));

	// render the entire scene
	rc = R_Main(framebuffer, w, h);

	// convert from floating point into our vec3ub
	for (i = 0; i < w; i++) {
		for (j = 0; j < h; j++) {
			idx = IDX2D(i, j, w);
			img[idx].r = framebuffer[idx].r * 255.0;
			img[idx].g = framebuffer[idx].g * 255.0;
			img[idx].b = framebuffer[idx].b * 255.0;
		}
	}

	rc = stbi_write_png("output.png", w, h, 3, img, 0);
	if (!rc) {
		fprintf(stderr, "Error, stbi_write_png failed\n");
		exit(1);
	}

	free(img);
	free(framebuffer);

	return 0;
}

/* R_Main : rendering main function */
int R_Main(vec3f *framebuffer, s32 w, s32 h)
{
	struct world_t *world;

	s32 i, j;

	vec3f camera_p, camera_x, camera_y, camera_z;

	vec3f film_p, film_c;
	f32 film_x, film_y, film_d;
	f32 film_w, film_h;
	f32 halffilm_w, halffilm_h;

	vec3f ray_origin;
	vec3f ray_dir;
	vec3f color;

	// construct the world first
	world = R_WorldGen();

	camera_p = M_Vec3f(0, 0, 10);
	camera_z = M_NormVec3f(camera_p);
	camera_x = M_NormVec3f(M_CrossVec3f(camera_z, M_Vec3f(0, 0, 1)));
	camera_y = M_NormVec3f(M_CrossVec3f(camera_z, camera_x));

	film_d = 1.0f;
	film_w = 1.0f;
	film_h = 1.0f;
	halffilm_w = film_w / 2.0;
	halffilm_h = film_h / 2.0;
	film_c = M_SubVec3f(camera_p, M_ScaleVec3f(camera_z, film_d));

	for (j = 0; j < h; j++) {
		film_y = -1.0f + 2.0f * ((f32)j / (f32)h);

		for (i = 0; i < w; i++) {

			film_x = -1.0f + 2.0f * ((f32)i / (f32)w);

			film_p = M_CopyVec3f(film_c);
			film_p = M_AddVec3f(film_p, M_ScaleVec3f(camera_x, film_x * halffilm_w));
			film_p = M_AddVec3f(film_p, M_ScaleVec3f(camera_y, film_y * halffilm_h));

			ray_origin = M_CopyVec3f(camera_p);
			ray_dir = M_NormVec3f(M_SubVec3f(film_p, camera_p));

			color = R_RayCast(world, ray_origin, ray_dir);

			framebuffer[i + j * w] = M_CopyVec3f(color);
		}
	}

	R_WorldFree(world);

	return 0;
}

/* R_RayCast : cast a ray into the world, returning the color vector */
vec3f R_RayCast(struct world_t *world, vec3f ray_origin, vec3f ray_direction)
{
	vec3f res;
	struct plane_t plane;
	f32 hitdist, thisdist;
	f32 tolerance;
	f32 denom;
	s32 i;

	tolerance = 0.0001f;

	hitdist = FLT_MAX;

	res = M_CopyVec3f(world->materials[0].color);

	// check for intersections with planes
	for (i = 0; i < world->planes_len; i++) {
		memcpy(&plane, world->planes + i, sizeof(plane));

		denom = M_DotVec3f(plane.n, ray_direction);
		if ((denom < -tolerance) || (tolerance < denom)) {
			thisdist = (-plane.d - M_DotVec3f(plane.n, ray_origin)) / denom;

			printf("hit %f\n", thisdist);
			if ((0.0f < thisdist) && (thisdist < hitdist)) {
				hitdist = thisdist;
				res = M_CopyVec3f(world->materials[plane.mat].color);
			}
		}
	}

	return res;
}

/* R_WorldGen : generates or loads the world up */
struct world_t *R_WorldGen()
{
	struct world_t *world;

	world = calloc(1, sizeof(*world));

	world->materials_len = 3;
	world->materials_cnt = 3;
	world->materials = calloc(world->materials_cnt, sizeof(*world->materials));

	world->planes_len = 1;
	world->planes_cnt = 1;
	world->planes = calloc(world->planes_cnt, sizeof(*world->planes));

	world->spheres_len = 3;
	world->spheres_cnt = 3;
	world->spheres = calloc(world->spheres_cnt, sizeof(*world->spheres));

	// careful here!!!!
	world->materials[0].color = M_Vec3f(0, 0, 0);
	world->materials[1].color = M_Vec3f(1, 1, 1);
	world->materials[2].color = M_Vec3f(0.9, 0.0, 0.1);

	world->planes[0].n = M_Vec3f(0, 0, 1);
	world->planes[0].d = 0;
	world->planes[0].mat = 1;

	return world;
}

/* R_WorldFree : frees the world structure */
void R_WorldFree(struct world_t *world)
{
	if (world) {
		if (world->planes) {
			free(world->planes);
		}
		if (world->spheres) {
			free(world->spheres);
		}
		if (world->materials) {
			free(world->materials);
		}
		free(world);
	}
}

