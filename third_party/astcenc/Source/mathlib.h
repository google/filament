/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012, 2018 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Internal math library declarations for ASTC codec.
 */
/*----------------------------------------------------------------------------*/

#ifndef MATHLIB_H_INCLUDED

#define MATHLIB_H_INCLUDED

#include "vectypes.h"

// basic OpenCL functions
float inversesqrt(float p);
float acospi(float p);
float sinpi(float p);
float cospi(float p);

float nan(int p);

#if !defined(_MSC_VER) && (__cplusplus < 201103L)
float fmax(float p, float q);
float fmin(float p, float q);
#endif  // C++11

float2 fmax(float2 p, float2 q);

float3 fmax(float3 p, float3 q);

float4 fmax(float4 p, float4 q);
float2 fmin(float2 p, float2 q);
float3 fmin(float3 p, float3 q);
float4 fmin(float4 p, float4 q);

/*
	float dot( float2 p, float2 q );
	float dot( float3 p, float3 q );
	float dot( float4 p, float4 q );
*/

static inline float dot(float2 p, float2 q)
{
	return p.x * q.x + p.y * q.y;
}
static inline float dot(float3 p, float3 q)
{
	return p.x * q.x + p.y * q.y + p.z * q.z;
}
static inline float dot(float4 p, float4 q)
{
	return p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w;
}


float3 cross(float3 p, float3 q);
float4 cross(float4 p, float4 q);

float length(float2 p);
float length(float3 p);
float length(float4 p);

float length_sqr(float2 p);
float length_sqr(float3 p);
float length_sqr(float4 p);

float distance(float2 p, float2 q);
float distance(float3 p, float3 q);
float distance(float4 p, float4 q);

float distance_sqr(float2 p, float2 q);
float distance_sqr(float3 p, float3 q);
float distance_sqr(float4 p, float4 q);

float2 normalize(float2 p);
float3 normalize(float3 p);
float4 normalize(float4 p);



// functions other than just basic OpenCL functions

float4 gcross(float4 p, float4 q, float4 r);

struct mat2
{
	float2 v[2];
};
struct mat3
{
	float3 v[3];
};
struct mat4
{
	float4 v[4];
};

float trace(mat2 p);
float trace(mat3 p);
float trace(mat4 p);

float determinant(mat2 p);
float determinant(mat3 p);
float determinant(mat4 p);

float2 characteristic_poly(mat2 p);
float3 characteristic_poly(mat3 p);
float4 characteristic_poly(mat4 p);

float2 solve_monic(float2 p);
float3 solve_monic(float3 p);
float4 solve_monic(float4 p);

float2 transform(mat2 p, float2 q);
float3 transform(mat3 p, float3 q);
float4 transform(mat4 p, float4 q);

mat2 adjugate(mat2 p);
mat3 adjugate(mat3 p);
mat4 adjugate(mat4 p);

mat2 invert(mat2 p);
mat3 invert(mat3 p);
mat4 invert(mat4 p);

float2 eigenvalues(mat2 p);
float3 eigenvalues(mat3 p);
float4 eigenvalues(mat4 p);

float2 eigenvector(mat2 p, float eigvl);
float3 eigenvector(mat3 p, float eigvl);
float4 eigenvector(mat4 p, float eigvl);

mat2 operator *(mat2 a, mat2 b);
mat3 operator *(mat3 a, mat3 b);
mat4 operator *(mat4 a, mat4 b);



// parametric line, 2D: The line is given by line = a + b*t.
struct line2
{
	float2 a;
	float2 b;
};

// parametric line, 3D
struct line3
{
	float3 a;
	float3 b;
};

struct line4
{
	float4 a;
	float4 b;
};

// plane/hyperplane defined by a point and a normal vector
struct plane_3d
{
	float3 root_point;
	float3 normal;				// normalized
};

struct hyperplane_4d
{
	float4 root_point;
	float4 normal;				// normalized
};

float param_nearest_on_line(float2 point, line2 line);
float param_nearest_on_line(float3 point, line3 line);
float param_nearest_on_line(float4 point, line4 line);

float point_line_distance(float2 point, line2 line);
float point_line_distance(float3 point, line3 line);
float point_line_distance(float4 point, line4 line);

float point_line_distance_sqr(float2 point, line2 line);
float point_line_distance_sqr(float3 point, line3 line);
float point_line_distance_sqr(float4 point, line4 line);

float point_plane_3d_distance(float3 point, plane_3d plane);
float point_hyperplane_4d_distance(float4 point, hyperplane_4d plane);

plane_3d generate_plane_from_points(float3 point0, float3 point1, float3 point2);
hyperplane_4d generate_hyperplane_from_points(float4 point0, float4 point1, float4 point2, float4 point3);


#endif
