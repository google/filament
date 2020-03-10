#version 450

layout(location = 0, component = 3) in float Foo1;
layout(location = 0, component = 0) in vec3 Foo3;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(Foo3, Foo1);
}
