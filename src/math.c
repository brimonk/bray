/*
 * Brian Chrzanowski
 * Wed Dec 11, 2019 14:46
 *
 * Brian's Math Library
 *
 * TODO (brian)
 * 1. redefine math.h stdlib to remove dependency
 */

#include <math.h>

#include "math.h"

/* M_CrossVec3i : returns the cross product of vector a and vector b */
vec3i M_CrossVec3i(vec3i a, vec3i b)
{
	vec3i c;

	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.x - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;

	return c;
}

/* M_CrossVec3f : returns the cross product of vector a and vector b */
vec3f M_CrossVec3f(vec3f a, vec3f b)
{
	vec3f c;

	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.x - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;

	return c;
}

/* M_NormVec3f : normalize a vec3f */
vec3f M_NormVec3f(vec3f n)
{
	vec3f r;
	f32 mag;

	mag = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);

	r.x = n.x / mag;
	r.y = n.y / mag;
	r.z = n.z / mag;

	return r;
}

/* M_Vec2ub : small constructor for vec2ub */
vec2ub M_Vec2ub(u8 a, u8 b)
{
	vec2ub r;

	r.x = a;
	r.y = b;

	return r;
}

/* M_Vec2i : small constructor for vec3i */
vec2i M_Vec2i(s32 a, s32 b)
{
	vec2i r;

	r.x = a;
	r.y = b;

	return r;
}

/* M_Vec2f : small constructor for vec3i */
vec2f M_Vec2f(f32 a, f32 b)
{
	vec2f r;

	r.x = a;
	r.y = b;

	return r;
}

/* M_Vec3ub : small constructor for vec3ub */
vec3ub M_Vec3ub(u8 a, u8 b, u8 c)
{
	vec3ub r;

	r.x = a;
	r.y = b;
	r.z = c;

	return r;
}

/* M_Vec3i : small constructor for vec3i */
vec3i M_Vec3i(s32 a, s32 b, s32 c)
{
	vec3i r;

	r.x = a;
	r.y = b;
	r.z = c;

	return r;
}

/* M_Vec3f : small constructor for vec3i */
vec3f M_Vec3f(f32 a, f32 b, f32 c)
{
	vec3f r;

	r.x = a;
	r.y = b;
	r.z = c;

	return r;
}

/* M_Absi : return the absolute value of a floating point number */
s32 M_Absi(s32 in)
{
	return in < 0.0 ? -in : in;
}

/* M_Absf : return the absolute value of a floating point number */
f32 M_Absf(f32 in)
{
	return in < 0.0 ? -in : in;
}

/* M_CopyVec3f : helper function to copy vec3f's */
vec3f M_CopyVec3f(vec3f a)
{
	vec3f r;

	r.x = a.x;
	r.y = a.y;
	r.z = a.z;

	return r;
}

/* M_CopyVec3i : helper function to copy vec3f's */
vec3i M_CopyVec3i(vec3i a)
{
	return a;
}

/* M_CopyVec3ub : helper function to copy vec3f's */
vec3ub M_CopyVec3ub(vec3ub a)
{
	return a;
}

/* M_DotVec3f : compute the dot product of two vectors */
f32 M_DotVec3f(vec3f a, vec3f b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

/* M_SubVec3f : subtract b from a */
vec3f M_SubVec3f(vec3f a, vec3f b)
{
	vec3f c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;

	return c;
}

/* M_ScaleVec3f : multiply a vector by a scalar */
vec3f M_ScaleVec3f(vec3f a, f32 b)
{
	vec3f r;

	r.x = a.x * b;
	r.y = a.y * b;
	r.z = a.z * b;

	return r;
}

/* M_AddVec3f : subtract b from a */
vec3f M_AddVec3f(vec3f a, vec3f b)
{
	vec3f r;

	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;

	return r;
}

/* M_DivVec3f : divide vector a by b */
vec3f M_DivVec3f(vec3f a, vec3f b)
{
	vec3f r;

	r.x = a.x / b.x;
	r.y = a.y / b.y;
	r.z = a.z / b.z;

	return r;
}

