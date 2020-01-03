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

#define EPSILON (0.0001f)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH      (1024)
#define HEIGHT     (768)
#define COMPONENTS (3)

struct model_t { // to read models in the wavefront format
	vecf3_t *v;
	vecf3_t *t;
	vecf3_t *n;
	veci3_t *indv;
	veci3_t *indt;
	veci3_t *indn;
	size_t len_v;
	size_t len_t;
	size_t len_n;
	size_t len_indv;
	size_t len_indt;
	size_t len_indn;
	size_t cap_v;
	size_t cap_t;
	size_t cap_n;
	size_t cap_indv;
	size_t cap_indt;
	size_t cap_indn;
};

struct triangle_t {
	vecf3_t a, b, c;
	vecf3_t n;
};

struct world_t {
	struct triangle_t *t;
	size_t t_cnt, t_len;
};

/* R_Main : rendering main function */
int R_Main(struct world_t *world, vecf3_t *framebuffer, s32 w, s32 h);

/* R_RayCast : cast a ray into the world, returning the color vector */
int R_RayCast(struct world_t *world, vecf3_t out, vecf3_t origin, vecf3_t dir);

/* R_IntersectTriangle : determines if a ray intersects with a triangle */
int R_IntersectTriangle(vecf3_t out, struct triangle_t *t, vecf3_t origin, vecf3_t dir);

/* A_WorldLoad : loads the entire world */
void A_WorldLoad(struct world_t **world);

/* A_WorldFree : frees the world */
void A_WorldFree(struct world_t *world);

/* A_LoadModel : processes a model file into the structure */
struct model_t *A_LoadModel(char *name);

/* A_FreeModel : all resources related to the model */
void A_FreeModel(struct model_t *model);

int main(int argc, char **argv)
{
	struct world_t *world;
	u8 *img;
	vecf3_t *framebuffer;
	s32 w, h, c;
	s32 i, j;
	s32 idx;
	int rc;

	w = WIDTH;
	h = HEIGHT;
	c = COMPONENTS;

	img = calloc(w * h * c, sizeof(*img));
	framebuffer = calloc(w * h, sizeof(*framebuffer));

	A_WorldLoad(&world);

	// render the entire scene
	rc = R_Main(world, framebuffer, w, h);

	// convert from floating point into our vec3ub
	for (i = 0; i < w; i++) {
		for (j = 0; j < h; j++) {
			idx = IDX3D(i, j, 0, w, h);
			img[idx + 0] = framebuffer[idx][0] * 255.0;
			img[idx + 1] = framebuffer[idx][1] * 255.0;
			img[idx + 2] = framebuffer[idx][2] * 255.0;
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
int R_Main(struct world_t *world, vecf3_t *framebuffer, s32 w, s32 h)
{
	s32 i, j;

	vecf3_t camera_p, camera_x, camera_y, camera_z;

	vecf3_t film_p, film_c;
	f32 film_x, film_y, film_d;
	f32 film_w, film_h;
	f32 halffilm_w, halffilm_h;

	vecf3_t color;

	vecf3_t origin, dir;
	vecf3_t tmp;

	// setup our camera position and coordinate system
	Vec3(camera_p, 0, -10, 1);

	Vec3Norm(camera_z, camera_p);
	Vec3(tmp, 0, 0, 1);
	Vec3Cross(camera_x, tmp, camera_z);
	Vec3Norm(camera_x, camera_x);
	Vec3Cross(camera_y, camera_x, camera_z);
	Vec3Norm(camera_y, camera_y);

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
	Vec3Scale(tmp, camera_z, film_d);
	Vec3Sub(film_c, camera_p, tmp);

	// copy the origin once because it's always the same
	Vec3Copy(origin, camera_p);

	for (j = 0; j < h; j++) {
		film_y = -1.0f + 2.0f * ((f32)j / (f32)h);

		for (i = 0; i < w; i++) {

			film_x = -1.0f + 2.0f * ((f32)i / (f32)w);

			Vec3Copy(film_p, film_c);
			Vec3Scale(tmp, camera_x, (film_x * halffilm_w));
			Vec3Add(film_p, film_p, tmp);
			Vec3Scale(tmp, camera_y, (film_y * halffilm_h));

			Vec3Sub(dir, film_p, camera_p);
			Vec3Norm(dir, dir);

			R_RayCast(world, color, origin, dir);

			framebuffer[i + j * w][0] = color[0];
			framebuffer[i + j * w][1] = color[1];
			framebuffer[i + j * w][2] = color[2];
		}
	}

	return 0;
}

/* R_RayCast : cast a ray into the world, returning the color vector */
int R_RayCast(struct world_t *world, vecf3_t out, vecf3_t origin, vecf3_t dir)
{
	size_t i;
	struct triangle_t *t;
	vecf3_t tuv; // literally the t, u, and v values from the intersection

	Vec3(out, 0, 0, 0);

	for (i = 0; i < world->t_len; i++) {
		t = world->t + i;

		if (R_IntersectTriangle(tuv, t, origin, dir)) {
			Vec3(out, 1, 1, 1);
		}
	}

	return 0;
}

/* R_IntersectTriangle : determines if a ray intersects with a triangle */
int R_IntersectTriangle(vecf3_t tuv, struct triangle_t *tri, vecf3_t origin, vecf3_t dir)
{
	vecf3_t edge1, edge2, tvec, pvec, qvec;
	f32 det, inv_det, t, u, v;

	Vec3(tuv, 0, 0, 0);

	// find vectors for two edges sharing vertex "a"
	Vec3Sub(edge1, tri->b, tri->a);
	Vec3Sub(edge2, tri->c, tri->a);

	// begin calculating the determinant - used to calculate 'u'
	Vec3Cross(pvec, dir, edge2);

	// if determinant is near zero, ray lies in the plane of the triangle
	det = Vec3Dot(edge1, pvec);

	if (det < EPSILON) {
		return 0;
	}

	// NOTE (brian) I only have the triangle culling path
	// calculate the distance from a to ray origin
	Vec3Sub(tvec, origin, tri->a);

	// calculate 'u' parameter and test bounds
	u = Vec3Dot(tvec, pvec);
	if (u < 0.0 || u > det) {
		return 0;
	}

	// prepare to test v
	Vec3Cross(qvec, tvec, edge1);

	// calculate v parameter and test bounds
	v = Vec3Dot(dir, qvec);
	if (v < 0.0 || u + v > det) {
		return 0;
	}

	// calculate 1, scale parameters, ray intersects triangle
	t = Vec3Dot(edge2, qvec);
	inv_det = 1.0 / det;
	t *= inv_det;
	u *= inv_det;
	v *= inv_det;

	// now put them into our output vecf3_t
	tuv[0] = t;
	tuv[1] = u;
	tuv[2] = v;

	return 1;
}

/* A_WorldLoad : loads the entire world */
void A_WorldLoad(struct world_t **world)
{
	struct world_t *w;

	if (world) {
		w = calloc(1, sizeof(*w));

		*world = w;
	}
}

/* A_WorldFree : frees the world */
void A_WorldFree(struct world_t *world)
{
	if (world) {
		free(world->t);
		free(world);
	}
}

/* A_LoadModel : processes a model file into the structure */
struct model_t *A_LoadModel(char *name)
{
	struct model_t *m;
	char *words[16];
	char *s;
	FILE *fp;
	char *cmd;
	char buf[BUFSMALL];
	s32 i;

	m = calloc(1, sizeof(struct model_t));
	fp = fopen(name, "r");

	if (m && fp) {
		// init arrays and such
		C_ArrayRealloc(m->v, &m->cap_v, &m->len_v, sizeof(*m->v));
		C_ArrayRealloc(m->t, &m->cap_t, &m->len_t, sizeof(*m->t));
		C_ArrayRealloc(m->n, &m->cap_n, &m->len_n, sizeof(*m->n));
		C_ArrayRealloc(m->indv, &m->cap_indv, &m->len_indv, sizeof(*m->indv));
		C_ArrayRealloc(m->indt, &m->cap_indn, &m->len_indn, sizeof(*m->indt));
		C_ArrayRealloc(m->indn, &m->cap_indt, &m->len_indt, sizeof(*m->indn));

		while (buf == fgets(buf, sizeof(buf), fp)) {
			buf[strlen(buf) - 1] = 0;

			if (strlen(buf) == 0 || buf[0] == '#') {
				continue;
			}

			// parse the line into space delimited words, up to # of words
			for (i = 0, s = strtok(buf, " /"); s && i < ARRSIZE(words); i++, s = strtok(NULL, " /")) {
				words[i] = s;
			}

			cmd = words[0];

			if (cmd[0] == 'v' && cmd[1] == '\0') {
				m->v[m->len_v][0] = atof(words[1]);
				m->v[m->len_v][1] = atof(words[2]);
				m->v[m->len_v][2] = atof(words[3]);
				m->len_v++;
			} else if (cmd[0] == 'v' && cmd[1] == 't') {
				m->t[m->len_t][0] = atof(words[1]);
				m->t[m->len_t][1] = atof(words[2]);
				m->t[m->len_t][2] = atof(words[3]);
				m->len_t++;
			} else if (cmd[0] == 'v' && cmd[1] == 'n') {
				m->n[m->len_n][0] = atof(words[1]);
				m->n[m->len_n][1] = atof(words[2]);
				m->n[m->len_n][2] = atof(words[3]);
				m->len_n++;
			} else if (cmd[0] == 'f' && cmd[1] == '\0') {
				// NOTE this case is special (nnn/nnn/nnn nnn/nnn/nnn nnn/nnn/nnn)
				m->indv[m->len_indv][0] = atoi(words[1]) - 1;
				m->indv[m->len_indv][1] = atoi(words[4]) - 1;
				m->indv[m->len_indv][2] = atoi(words[7]) - 1;

				m->indt[m->len_indt][0] = atoi(words[2]) - 1;
				m->indt[m->len_indt][1] = atoi(words[5]) - 1;
				m->indt[m->len_indt][2] = atoi(words[8]) - 1;

				m->indn[m->len_indn][0] = atoi(words[3]) - 1;
				m->indn[m->len_indn][1] = atoi(words[6]) - 1;
				m->indn[m->len_indn][2] = atoi(words[9]) - 1;

				m->len_indv++; // update the indicies
				m->len_indt++;
				m->len_indn++;
			}

			// realloc where needed
			C_ArrayRealloc(m->v, &m->cap_v, &m->len_v, sizeof(*m->v));
			C_ArrayRealloc(m->t, &m->cap_t, &m->len_t, sizeof(*m->t));
			C_ArrayRealloc(m->n, &m->cap_n, &m->len_n, sizeof(*m->n));
			C_ArrayRealloc(m->indv, &m->cap_indv, &m->len_indv, sizeof(*m->indv));
			C_ArrayRealloc(m->indt, &m->cap_indn, &m->len_indn, sizeof(*m->indt));
			C_ArrayRealloc(m->indn, &m->cap_indt, &m->len_indt, sizeof(*m->indn));
		}

		fclose(fp);
	}

	return m;
}

/* A_FreeModel : all resources related to the model */
void A_FreeModel(struct model_t *model)
{
	if (model) {
		free(model->v);
		free(model->t);
		free(model->n);
		free(model->indv);
		free(model->indt);
		free(model->indn);
		free(model);
	}
}

