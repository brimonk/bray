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

typedef union vec2ub
{
	struct
	{
		u8 x, y;
	};

	struct
	{
		u8 u, v;
	};

	struct
	{
		u8 w, h;
	};

	u8 e[2];

} vec2ub;

typedef union vec2i
{
	struct
	{
		s32 x, y;
	};

	struct
	{
		s32 u, v;
	};

	struct
	{
		s32 w, h;
	};

	s32 e[2];

} vec2i;

typedef union vec3ub
{
	struct
	{
		u8 x, y, z;
	};

	struct
	{
		u8 u, v, w;
	};

	struct
	{
		u8 r, g, b;
	};

	u8 e[3];

} vec3ub;

typedef union vec3i
{
	struct
	{
		s32 x, y, z;
	};

	struct
	{
		s32 u, v, w;
	};

	struct
	{
		s32 r, g, b;
	};

	s32 e[3];

} vec3i;

typedef union vec4i
{
	struct
	{
		s32 x, y, z, w;
	};

	struct
	{
		s32 r, g, b, a;
	};

	s32 e[4];

} vec4i;

typedef union vec2f
{
	struct
	{
		f32 x, y;
	};

	struct
	{
		f32 u, v;
	};

	struct
	{
		f32 w, h;
	};

	f32 e[2];

} vec2f;

typedef union vec3f
{
	struct
	{
		f32 x, y, z;
	};

	struct
	{
		f32 u, v, w;
	};

	struct
	{
		f32 r, g, b;
	};

	f32 e[3];

} vec3f;

typedef union vec4f
{
	struct
	{
		f32 x, y, z, w;
	};

	struct
	{
		f32 r, g, b, a;
	};

	f32 e[4];

} vec4f;

typedef union mat4i
{
	s32 es[4][4];
} mat4i;

typedef union mat4f
{
	f32 e[4][4];
} mat4f;

typedef union quati
{
	struct
	{
		s32 x, y, z, w;
	};
	s32 e[4];
} quati;

/*
 * Math Function Definition Lie Below Here
 */

/* M_Sin : returns the sin of the angle */
f32 M_Sin(f32 theta);

/* M_Cos : returns the cos of the angle */
f32 M_Cos(f32 theta);

/* M_Absi : return the absolute value of a floating point number */
s32 M_Absi(s32 in);

/* M_Absf : return the absolute value of a floating point number */
f32 M_Absf(f32 in);

/* M_Vec2ub : small constructor for vec2ub */
vec2ub M_Vec2ub(u8 a, u8 b);

/* M_Vec2i : small constructor for vec3i */
vec2i M_Vec2i(s32 a, s32 b);

/* M_Vec2f : small constructor for vec3i */
vec2f M_Vec2f(f32 a, f32 b);

/* M_Vec3ub : small constructor for vec3ub */
vec3ub M_Vec3ub(u8 a, u8 b, u8 c);

/* M_Vec3i : small constructor for vec3i */
vec3i M_Vec3i(s32 a, s32 b, s32 c);

/* M_Vec3f : small constructor for vec3i */
vec3f M_Vec3f(f32 a, f32 b, f32 c);

/* M_DotVec3f : multiply two vectors */
f32 M_DotVec3f(vec3f a, vec3f b);

/* M_ScaleVec3f : multiply a vector by a scalar */
vec3f M_ScaleVec3f(vec3f a, f32 b);

/* M_CrossVec3i : returns the cross product of vector a and vector b */
vec3i M_CrossVec3i(vec3i a, vec3i b);

/* M_CrossVec3f : returns the cross product of vector a and vector b */
vec3f M_CrossVec3f(vec3f a, vec3f b);

/* M_CopyVec3f : helper function to copy vec3f's */
vec3f M_CopyVec3f(vec3f a);

/* M_CopyVec3i : helper function to copy vec3f's */
vec3i M_CopyVec3i(vec3i a);

/* M_CopyVec3ub : helper function to copy vec3f's */
vec3ub M_CopyVec3ub(vec3ub a);

/* M_NormVec3f : normalize a vec3f */
vec3f M_NormVec3f(vec3f n);

/* M_SubVec3f : subtract b from a */
vec3f M_SubVec3f(vec3f a, vec3f b);

/* M_AddVec3f : subtract b from a */
vec3f M_AddVec3f(vec3f a, vec3f b);

/* M_DivVec3f : divide vector a by b */
vec3f M_DivVec3f(vec3f a, vec3f b);

/* M_Sqrt : returns the square root of the input value */
f32 M_Sqrt(f32 in);

#endif // MATH_H

