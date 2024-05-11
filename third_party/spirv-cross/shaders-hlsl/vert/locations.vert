#version 310 es
#extension GL_EXT_shader_io_blocks : require

struct Foo
{
	vec3 a;
	vec3 b;
	vec3 c;
};

// This will lock to input location 2.
layout(location = 2) in vec4 Input2;
// This will lock to input location 4.
layout(location = 4) in vec4 Input4;
// This will pick first available, which is 0.
layout(location = 0) in vec4 Input0;

// Locks output 0.
layout(location = 0) out float vLocation0;
// Locks output 1.
layout(location = 1) out float vLocation1;
// Picks first available two locations, so, 2 and 3.
layout(location = 2) out float vLocation2[2];
// Picks first available location, 4.
layout(location = 4) out Foo vLocation4;
// Picks first available location 9.
layout(location = 9) out float vLocation9;

// Locks location 7 and 8.
layout(location = 7) out VertexOut
{
	vec3 color;
	vec3 foo;
} vout;

void main()
{
	gl_Position = vec4(1.0) + Input2 + Input4 + Input0;
	vLocation0 = 0.0;
	vLocation1 = 1.0;
	vLocation2[0] = 2.0;
	vLocation2[1] = 2.0;
	Foo foo;
	foo.a = vec3(1.0);
	foo.b = vec3(1.0);
	foo.c = vec3(1.0);
	vLocation4 = foo;
	vLocation9 = 9.0;
	vout.color = vec3(2.0);
	vout.foo = vec3(4.0);
}
