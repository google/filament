#version 460

layout(location = 0) out TDPickVertex
{
vec4 c;
vec3 uv[1];
} oTDVert;

layout(location = 0) in vec3 P;
layout(location = 1) in vec3 uv[1];

void main()
{
gl_Position = vec4(P, 1.0);
oTDVert.uv[0] = uv[0];
oTDVert.c = vec4(1.);
}