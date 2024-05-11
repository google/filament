#version 310 es

uniform vec4 UBO[56];
layout(location = 0) in vec4 aVertex;

void main()
{
    gl_Position = ((mat4(UBO[40], UBO[41], UBO[42], UBO[43]) * aVertex) + UBO[55]) + ((UBO[50] + UBO[45]) + vec4(UBO[54].x));
}

