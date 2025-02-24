#version 450

#include "glsl.-P.included.glsl"

layout(location=0) out vec4 color;

void main()
{
    color = getColor();
}
