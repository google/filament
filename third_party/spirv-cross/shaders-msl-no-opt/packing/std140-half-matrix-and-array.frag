#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(set = 0, binding = 0, std140) uniform Foo
{
	f16mat2x2 c22;
	f16mat2x2 c22arr[3];
	f16mat2x3 c23;
	f16mat2x4 c24;

	f16mat3x2 c32;
	f16mat3x3 c33;
	f16mat3x4 c34;

	f16mat4x2 c42;
	f16mat4x3 c43;
	f16mat4x4 c44;

	layout(row_major) f16mat2x2 r22;
	layout(row_major) f16mat2x2 r22arr[3];
	layout(row_major) f16mat2x3 r23;
	layout(row_major) f16mat2x4 r24;

	layout(row_major) f16mat3x2 r32;
	layout(row_major) f16mat3x3 r33;
	layout(row_major) f16mat3x4 r34;

	layout(row_major) f16mat4x2 r42;
	layout(row_major) f16mat4x3 r43;
	layout(row_major) f16mat4x4 r44;

	float16_t h1[6];
	f16vec2 h2[6];
	f16vec3 h3[6];
	f16vec4 h4[6];
} u;

layout(location = 0) out vec4 FragColor;

void main()
{
	// Load vectors.
	f16vec2 c2 = u.c22[0] + u.c22[1];
	c2 = u.c22arr[2][0] + u.c22arr[2][1];
	f16vec3 c3 = u.c23[0] + u.c23[1];
	f16vec4 c4 = u.c24[0] + u.c24[1];

	c2 = u.c32[0] + u.c32[1] + u.c32[2];
	c3 = u.c33[0] + u.c33[1] + u.c33[2];
	c4 = u.c34[0] + u.c34[1] + u.c34[2];

	c2 = u.c42[0] + u.c42[1] + u.c42[2] + u.c42[3];
	c3 = u.c43[0] + u.c43[1] + u.c43[2] + u.c43[3];
	c4 = u.c44[0] + u.c44[1] + u.c44[2] + u.c44[3];

	// Load scalars.
	float16_t c = u.c22[0].x + u.c22[0].y + u.c22[1].x + u.c22[1].y;
	c = u.c22arr[2][0].x + u.c22arr[2][0].y + u.c22arr[2][1].x + u.c22arr[2][1].y;

	// Load full matrix.
	f16mat2x2 c22 = u.c22;
	c22 = u.c22arr[2];
	f16mat2x3 c23 = u.c23;
	f16mat2x4 c24 = u.c24;

	f16mat3x2 c32 = u.c32;
	f16mat3x3 c33 = u.c33;
	f16mat3x4 c34 = u.c34;

	f16mat4x2 c42 = u.c42;
	f16mat4x3 c43 = u.c43;
	f16mat4x4 c44 = u.c44;

	// Same, but row-major.
	f16vec2 r2 = u.r22[0] + u.r22[1];
	r2 = u.r22arr[2][0] + u.r22arr[2][1];
	f16vec3 r3 = u.r23[0] + u.r23[1];
	f16vec4 r4 = u.r24[0] + u.r24[1];

	r2 = u.r32[0] + u.r32[1] + u.r32[2];
	r3 = u.r33[0] + u.r33[1] + u.r33[2];
	r4 = u.r34[0] + u.r34[1] + u.r34[2];

	r2 = u.r42[0] + u.r42[1] + u.r42[2] + u.r42[3];
	r3 = u.r43[0] + u.r43[1] + u.r43[2] + u.r43[3];
	r4 = u.r44[0] + u.r44[1] + u.r44[2] + u.r44[3];

	// Load scalars.
	float16_t r = u.r22[0].x + u.r22[0].y + u.r22[1].x + u.r22[1].y;

	// Load full matrix.
	f16mat2x2 r22 = u.r22;
	f16mat2x3 r23 = u.r23;
	f16mat2x4 r24 = u.r24;

	f16mat3x2 r32 = u.r32;
	f16mat3x3 r33 = u.r33;
	f16mat3x4 r34 = u.r34;

	f16mat4x2 r42 = u.r42;
	f16mat4x3 r43 = u.r43;
	f16mat4x4 r44 = u.r44;

	float16_t h1 = u.h1[5];
	f16vec2 h2 = u.h2[5];
	f16vec3 h3 = u.h3[5];
	f16vec4 h4 = u.h4[5];

	FragColor = vec4(1.0);
}
