#version 450

layout(location = 0) in vec2 v2a;
layout(location = 1) in vec2 v2b;
layout(location = 2) in vec3 v3a;
layout(location = 3) in vec3 v3b;
layout(location = 4) in vec4 v4a;
layout(location = 5) in vec4 v4b;

layout(location = 0) out mat2 m22;
layout(location = 2) out mat3 m33;
layout(location = 5) out mat4 m44;

void main()
{
	m22 = outerProduct(v2a, v2b);
	m33 = outerProduct(v3a, v3b);
	m44 = outerProduct(v4a, v4b);
}
