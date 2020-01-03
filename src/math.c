/*
 * Brian Chrzanowski
 * Wed Dec 11, 2019 14:46
 *
 * Brian's Math Library
 *
 * TODO (brian)
 * 1. redefine math.h stdlib to remove dependency
 */

#include <stdlib.h>

#include <math.h>

#include "math.h"

/* Vecf3Norm : normalize a vec3_t */
void Vec3Norm(vecf3_t out, vecf3_t in)
{
	f32 m;

	m = sqrt(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);

	out[0] /= m;
	out[1] /= m;
	out[2] /= m;
}

