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

#define FLOAT_TOLERANCE (0.0001f)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH  (1024)
#define HEIGHT (768)

struct material_t {
	vec3f color;
};

struct ray_t {
	vec3f origin;
	vec3f dir;
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
vec3f R_RayCast(struct world_t *world, struct ray_t *ray);

/* R_RayCrossPlane : checks if a ray crosses the given */
int R_RayCrossPlane(struct plane_t *plane, vec3f origin, vec3f dir, f32 *t);

/* R_RayCrossSphere : checks if a ray crosses the given sphere */
int R_RayCrossSphere(struct sphere_t *sphere, vec3f origin, vec3f dir, f32 *t);

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
	struct ray_t ray;

	s32 i, j;

	vec3f camera_p, camera_x, camera_y, camera_z;

	vec3f film_p, film_c;
	f32 film_x, film_y, film_d;
	f32 film_w, film_h;
	f32 halffilm_w, halffilm_h;

	vec3f color;

	// construct the world first
	world = R_WorldGen();

	camera_p = M_Vec3f(0, -10, 1);
	camera_z = M_NormVec3f(camera_p);
	camera_x = M_NormVec3f(M_CrossVec3f(M_Vec3f(0, 0, 1), camera_z));
	camera_y = M_NormVec3f(M_CrossVec3f(camera_x, camera_z));

	film_d = 1.0f;
	film_w = 1.0f;
	film_h = 1.0f;

	// alter film_w and film_h
	if (w > h) {
		film_h = film_w * ((f32)h / (f32)w);
	} else if (h > w) {
		film_w = film_h * ((f32)w / (f32)h);
	}

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

			ray.origin = M_CopyVec3f(camera_p);
			ray.dir = M_SubVec3f(film_p, camera_p);
			ray.dir = M_NormVec3f(ray.dir);

			color = R_RayCast(world, &ray);

			framebuffer[i + j * w] = M_CopyVec3f(color);
		}
	}

	R_WorldFree(world);

	return 0;
}

/* R_RayCast : cast a ray into the world, returning the color vector */
vec3f R_RayCast(struct world_t *world, struct ray_t *ray)
{
	struct plane_t *plane;
	struct sphere_t *sphere;
	f32 t;
	vec3f res;
	s32 i;
	int rc;

	t = FLT_MAX;

	res = M_CopyVec3f(world->materials[0].color);

	// check for intersections with planes
	for (i = 0; i < world->planes_len; i++) {
		plane = world->planes + i;
		rc = R_RayCrossPlane(plane, ray->origin, ray->dir, &t);

		if (rc) {
			res = M_CopyVec3f(world->materials[plane->mat].color);
		}
	}

	// check for intersections with planes
	for (i = 0; i < world->spheres_len; i++) {
		sphere = world->spheres + i;
		rc = R_RayCrossSphere(sphere, ray->origin, ray->dir, &t);

		if (rc) {
			res = M_CopyVec3f(world->materials[sphere->mat].color);
		}
	}

	return res;
}

/* R_RayCrossPlane : checks if a ray crosses the given */
int R_RayCrossPlane(struct plane_t *plane, vec3f origin, vec3f dir, f32 *t)
{
	f32 denom;
	f32 thisdist;

	denom = M_DotVec3f(plane->n, dir);
	if ((denom < -FLOAT_TOLERANCE) || (FLOAT_TOLERANCE < denom)) {
		thisdist = (-plane->d - M_DotVec3f(plane->n, origin)) / denom;

		if ((0.0f < thisdist) && (thisdist < *t)) {
			*t = thisdist;
			return true;
		}
	}

	return false;
}

/* R_RayCrossSphere : checks if a ray crosses the given sphere */
int R_RayCrossSphere(struct sphere_t *sphere, vec3f origin, vec3f dir, f32 *t)
{
	vec3f localorigin;
	f32 a, b, c;
	f32 tn, tp, res;
	f32 denom, rootterm;

	/*
	 * Uses the quadratic formula to solve for a sphere
	 * TODO (brian) document the math here.
	 */

	localorigin = M_SubVec3f(origin, sphere->p);

	a = M_DotVec3f(dir, dir);
	b = 2.0f * M_DotVec3f(dir, localorigin);
	c = M_DotVec3f(localorigin, localorigin) - sphere->r * sphere->r;

	denom = 2.0f * a;
	rootterm = M_Sqrt(b * b - 4.0f * a * c);

	if (rootterm > FLOAT_TOLERANCE) {
		tp = (-b + rootterm) / denom;
		tn = (-b - rootterm) / denom;

		printf("t = %f, tp = %f, tn = %f\n", (*t), tp, tn);

		res = tp;

		if (0 < tn && tn < tp) {
			res = tn;
		}

		if (res < *t) {
			*t = res;
			return true;
		}
	}

	return false;
}

/* R_WorldGen : generates or loads the world up */
struct world_t *R_WorldGen()
{
	struct world_t *world;

	world = calloc(1, sizeof(*world));

	world->materials_len = 4;
	world->materials_cnt = 4;
	world->materials = calloc(world->materials_cnt, sizeof(*world->materials));

	world->planes_len = 1;
	world->planes_cnt = 1;
	world->planes = calloc(world->planes_cnt, sizeof(*world->planes));

	world->spheres_len = 3;
	world->spheres_cnt = 3;
	world->spheres = calloc(world->spheres_cnt, sizeof(*world->spheres));

	// careful here!!!!
	world->materials[0].color = M_Vec3f(0, 0, 1);
	world->materials[1].color = M_Vec3f(1, 0, 0);
	world->materials[2].color = M_Vec3f(0, 0.6, 0.2);
	world->materials[2].color = M_Vec3f(0.1, 0.0, 0.5);

	world->planes[0].n = M_Vec3f(0, 0, 1);
	world->planes[0].d = 0;
	world->planes[0].mat = 1;

	world->spheres[0].p = M_Vec3f(0, 0, 0);
	world->spheres[0].r = 1.0f;
	world->spheres[0].mat = 2;

	world->spheres[1].p = M_Vec3f(-3, 2, 1);
	world->spheres[1].r = 1.5f;
	world->spheres[1].mat = 2;

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

