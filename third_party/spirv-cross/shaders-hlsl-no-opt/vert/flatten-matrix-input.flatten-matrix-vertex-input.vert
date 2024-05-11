#version 450

layout(location = 0) in mat4 m4;
layout(location = 4) in mat3 m3;
layout(location = 7) in mat2 m2;
layout(location = 9) in vec4 v;

void main()
{
	gl_Position = m4 * v;
	gl_Position.xyz += m3 * v.xyz;
	gl_Position.xy += m2 * v.xy;
}
