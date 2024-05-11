#version 450
layout(vertices = 4) out;

struct Meep
{
	float a;
	float b;
};

layout(location = 0) out float a[][2];
layout(location = 2) out float b[];
layout(location = 3) out mat2 m[];
layout(location = 5) out Meep meep[];
layout(location = 7) out Meep meeps[][2];

layout(location = 11) out Block
{
	float a[2];
	float b;
	mat2 m;
	Meep meep;
	Meep meeps[2];
} B[];

layout(location = 0) in float in_a[][2];
layout(location = 2) in float in_b[];
layout(location = 3) in mat2 in_m[];
layout(location = 5) in Meep in_meep[];

layout(location = 11) in Block
{
	float a[2];
	float b;
	mat2 m;
	// Non-multi-patch path cannot support structs inside structs.
} in_B[];

void write_in_func()
{
	gl_out[gl_InvocationID].gl_Position = vec4(1.0);

	a[gl_InvocationID][0] = in_a[gl_InvocationID][0];
	a[gl_InvocationID][1] = in_a[gl_InvocationID][1];
	b[gl_InvocationID] = in_b[gl_InvocationID];
	m[gl_InvocationID] = in_m[gl_InvocationID];
	meep[gl_InvocationID].a = in_meep[gl_InvocationID].a;
	meep[gl_InvocationID].b = in_meep[gl_InvocationID].b;
	meeps[gl_InvocationID][0].a = 1.0;
	meeps[gl_InvocationID][0].b = 2.0;
	meeps[gl_InvocationID][1].a = 3.0;
	meeps[gl_InvocationID][1].b = 4.0;

	B[gl_InvocationID].a[0] = in_B[gl_InvocationID].a[0];
	B[gl_InvocationID].a[1] = in_B[gl_InvocationID].a[1];
	B[gl_InvocationID].b = in_B[gl_InvocationID].b;
	B[gl_InvocationID].m = in_B[gl_InvocationID].m;
	B[gl_InvocationID].meep.a = 10.0;
	B[gl_InvocationID].meep.b = 20.0;
	B[gl_InvocationID].meeps[0].a = 5.0;
	B[gl_InvocationID].meeps[0].b = 6.0;
	B[gl_InvocationID].meeps[1].a = 7.0;
	B[gl_InvocationID].meeps[1].b = 8.0;
}

void main()
{
	write_in_func();
}
