#version 450

layout(location = 0) in mediump vec4 F;
layout(location = 1) flat in mediump ivec4 I;
layout(location = 2) flat in mediump uvec4 U;
layout(location = 0) out mediump vec4 FragColor;

void main()
{
	FragColor = F + vec4(I) + vec4(U);
}
