#version 450
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(set = 0, binding = 0, std140) buffer Foo
{
	f16mat2x3 c23;
	f16mat3x2 c32;
	layout(row_major) f16mat2x3 r23;
	layout(row_major) f16mat3x2 r32;

	float16_t h1[6];
	f16vec2 h2[6];
	f16vec3 h3[6];
	f16vec4 h4[6];
};

layout(location = 0) out vec4 FragColor;

void main()
{
	// Store scalar
	c23[1][2] = 1.0hf;
	c32[2][1] = 2.0hf;
	r23[1][2] = 3.0hf;
	r32[2][1] = 4.0hf;

	// Store vector
	c23[1] = f16vec3(0, 1, 2);
	c32[1] = f16vec2(0, 1);
	r23[1] = f16vec3(0, 1, 2);
	r32[1] = f16vec2(0, 1);

	// Store matrix
	c23 = f16mat2x3(1, 2, 3, 4, 5, 6);
	c32 = f16mat3x2(1, 2, 3, 4, 5, 6);
	r23 = f16mat2x3(1, 2, 3, 4, 5, 6);
	r32 = f16mat3x2(1, 2, 3, 4, 5, 6);

	// Store array
	h1[5] = 1.0hf;
	h2[5] = f16vec2(1, 2);
	h3[5] = f16vec3(1, 2, 3);
	h4[5] = f16vec4(1, 2, 3, 4);

	// Store scalar in array
	h2[5][1] = 10.0hf;
	h3[5][2] = 11.0hf;
	h4[5][3] = 12.0hf;

	FragColor = vec4(1.0);
}
