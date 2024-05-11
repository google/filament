#version 450
layout(vertices = 4) out;

struct Meep
{
	float a;
	float b;
};

layout(location = 0) patch out float a[2];
layout(location = 2) patch out float b;
layout(location = 3) patch out mat2 m;
layout(location = 5) patch out Meep meep;
layout(location = 7) patch out Meep meeps[2];

layout(location = 11) patch out Block
{
	float a[2];
	float b;
	mat2 m;
	Meep meep;
	Meep meeps[2];
} B;

void write_in_func()
{
	gl_out[gl_InvocationID].gl_Position = vec4(1.0);

	a[0] = 1.0;
	a[1] = 2.0;
	b = 3.0;
	m = mat2(2.0);
	meep.a = 4.0;
	meep.b = 5.0;
	meeps[0].a = 6.0;
	meeps[0].b = 7.0;
	meeps[1].a = 8.0;
	meeps[1].b = 9.0;

	B.a[0] = 1.0;
	B.a[1] = 2.0;
	B.b = 3.0;
	B.m = mat2(4.0);
	B.meep.a = 4.0;
	B.meep.b = 5.0;
	B.meeps[0].a = 6.0;
	B.meeps[0].b = 7.0;
	B.meeps[1].a = 8.0;
	B.meeps[1].b = 9.0;
}

void main()
{
	write_in_func();
}
