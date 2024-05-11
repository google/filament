#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 F;
layout(location = 1) flat in ivec4 I;
layout(location = 2) flat in uvec4 U;

void main()
{
    FragColor = (F + vec4(I)) + vec4(U);
}

