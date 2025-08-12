#version 430 core
#extension GL_ARB_enhanced_layouts : require

layout(vertices = 1) out;

layout (location = 1, component = 0)  in  double gohan[];
layout (location = 1, component = 2)  in  float goten[];


in  vec4 vs_tcs[];
out vec4 tcs_tes[];

void main()
{
}