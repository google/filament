#version 450

layout(location = 0) in vec4 vFoo;
layout(location = 0) out vec3 Foo3;
layout(location = 0, component = 3) out float Foo1;

void main()
{
	gl_Position = vFoo;
	Foo3 = vFoo.xyz;
	Foo1 = vFoo.w;
}
