/*
 * Brian Chrzanowski
 * Tue Dec 24, 2019 17:28
 *
 * Brian's Raytracer
 *
 * TODO
 * 1. Convert the Raytracer from the Handmade Ray inspired one, to
 *    operate using triangles only.
 *    Use Moeller-Trumbore ray-triangle intersection.
 *
 * 2. Load Models from Wavefront (obj) Format
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
	vec3f color_emit;
	vec3f color_reflect;
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
	size_t materials_cnt;
	size_t materials_len;
	struct plane_t *planes;
	size_t planes_cnt;
	size_t planes_len;
	struct sphere_t *spheres;
	size_t spheres_cnt;
	size_t spheres_len;
};

/* R_Main : rendering main function */
int R_Main(vec3f *framebuffer, s32 w, s32 h);

/* R_RayCast : cast a ray into the world, returning the color vector */
vec3f R_RayCast(struct world_t *world, struct ray_t *ray);

/* R_RayCrossPlane : checks if a ray crosses the given */
int R_RayCrossPlane(struct plane_t *plane, vec3f origin, vec3f dir, f32 *t);

/* R_RayCrossSphere : checks if a ray crosses the given sphere */
int R_RayCrossSphere(struct sphere_t *sphere, vec3f origin, vec3f dir, f32 *t);

/* A_WorldGen : generates or loads the world up */
struct world_t *A_WorldGen();

/* A_WorldFree : frees the world structure */
void A_WorldFree(struct world_t *world);

/* A_AddMaterial : adds a sphere into the world */
void A_AddMaterial(struct world_t *world, vec3f emit, vec3f reflect);

/* A_AddPlane : adds a sphere into the world */
void A_AddPlane(struct world_t *world, vec3f pos, f32 d, u32 mat);

/* A_AddSphere : adds a sphere into the world */
void A_AddSphere(struct world_t *world, vec3f pos, f32 r, u32 mat);

/* A_ArrayRealloc : reallocates an array if needed */
void A_ArrayRealloc(void *p, size_t *cnt, size_t *len, size_t elem);

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
	world = A_WorldGen();

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

	A_WorldFree(world);

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

	res = M_CopyVec3f(world->materials[0].color_emit);

	// check for intersections with planes
	for (i = 0; i < world->planes_len; i++) {
		plane = world->planes + i;
		rc = R_RayCrossPlane(plane, ray->origin, ray->dir, &t);

		if (rc) {
			res = M_CopyVec3f(world->materials[plane->mat].color_emit);
		}
	}

	// check for intersections with planes
	for (i = 0; i < world->spheres_len; i++) {
		sphere = world->spheres + i;
		rc = R_RayCrossSphere(sphere, ray->origin, ray->dir, &t);

		if (rc) {
			res = M_CopyVec3f(world->materials[sphere->mat].color_emit);
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

/* A_WorldGen : generates or loads the world up */
struct world_t *A_WorldGen()
{
	struct world_t *world;

	world = calloc(1, sizeof(*world));

	A_AddMaterial(world, M_Vec3f(1.0, 0.9, 0.9), M_Vec3f(0, 0, 0)); // sky
	A_AddMaterial(world, M_Vec3f(1.0, 1.0, 1.0), M_Vec3f(0, 0, 0)); // ground mat
	A_AddMaterial(world, M_Vec3f(0.0, 0.0, 1.0), M_Vec3f(0, 0, 1)); // sphere blue

	A_AddPlane(world, M_Vec3f(0, 0, 1), 0, 1); // ground

	A_AddSphere(world, M_Vec3f(0, 0, 0), 2, 2); // sphere

	return world;
}

/* A_WorldFree : frees the world structure */
void A_WorldFree(struct world_t *world)
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

/* A_AddMaterial : adds a sphere into the world */
void A_AddMaterial(struct world_t *world, vec3f emit, vec3f reflect)
{
	size_t t;

	if (!world) { // ERROR
		return;
	}

	A_ArrayRealloc(&world->materials, &world->materials_cnt, &world->materials_len, sizeof(struct material_t));

	t = world->materials_len++;

	world->materials[t].color_emit = M_CopyVec3f(emit);
	world->materials[t].color_reflect = M_CopyVec3f(reflect);
}

/* A_AddPlane : adds a sphere into the world */
void A_AddPlane(struct world_t *world, vec3f pos, f32 d, u32 mat)
{
	size_t t;

	if (!world) { // ERROR
		return;
	}

	A_ArrayRealloc(&world->planes, &world->planes_cnt, &world->planes_len, sizeof(struct plane_t));

	t = world->planes_len++;

	world->planes[t].n = M_CopyVec3f(pos);
	world->planes[t].d = d;
	world->planes[t].mat = mat;
}

/* A_AddSphere : adds a sphere into the world */
void A_AddSphere(struct world_t *world, vec3f pos, f32 r, u32 mat)
{
	size_t t;

	if (!world) { // ERROR
		return;
	}

	A_ArrayRealloc(&world->spheres, &world->spheres_cnt, &world->spheres_len, sizeof(struct sphere_t));

	t = world->spheres_len++;

	world->spheres[t].p = M_CopyVec3f(pos);
	world->spheres[t].r = r;
	world->spheres[t].mat = mat;
}

/* A_ArrayRealloc : reallocates an array if needed */
void A_ArrayRealloc(void *p, size_t *cnt, size_t *len, size_t elem)
{
	// NOTE (brian)
	// 1. this doesn't handle realloc failures
	// 2. we ASSUME (so the compiler will can it) that you PASS IN a
	//    void **-like thing. You have been warned.

	void **v;

	v = (void **)p;

	if (*cnt == *len) {
		if (*cnt) {
			*cnt *= 2;
		} else {
			*cnt = BUFSMALL;
		}
		*v = realloc(*v, elem * *cnt);
	}
}

