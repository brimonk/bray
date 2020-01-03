#ifndef MATH_H
#define MATH_H

/*
 * Brian Chrzanowski
 * Wed Dec 11, 2019 14:46
 *
 * Brian's Math Library
 *
 * TODO (brian)
 * 1. organize the typedefs for vectors and matricies
 */

#include "common.h"

#define MAX(x,y)    ((x) > (y) ? (x) : (y))
#define MIN(x,y)    ((x) < (y) ? (x) : (y))

typedef f32 vecf_t;
typedef vecf_t vecf2_t [2];
typedef vecf_t vecf3_t [3];
typedef vecf_t vecf4_t [4];
typedef s32 veci_t;
typedef veci_t veci2_t [2];
typedef veci_t veci3_t [3];
typedef veci_t veci4_t [4];

#define Vec3Cross(d,a,b) \
	((d)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1],\
	 (d)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2],\
	 (d)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0])

#define Vec3Dot(a,b)   ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define Vec3Add(d,a,b) ((d[0]=(a)[0]+(b)[0],d[1]=(a)[1]+(b)[1],d[2]=(a)[2]+(b)[2]))
#define Vec3Sub(d,a,b) ((d[0]=(a)[0]-(b)[0],d[1]=(a)[1]-(b)[1],d[2]=(a)[2]-(b)[2]))
#define Vec3(d,a,b,c)  (d[0]=(a),d[1]=(b),d[2]=(c))
#define Vec3Scale(d,v,s) ((d)[0]=(v)[0]*s,(d)[1]=(v)[1]*s,(d)[2]=(v)[2]*s)
#define Vec3Copy(d,v)  ((d)[0]=(v)[0],(d)[1]=(v)[1],(d)[2]=(v)[2])

/* Vecf3Norm : normalize a vec3_t */
void Vec3Norm(vecf3_t out, vecf3_t in);

#endif // MATH_H

