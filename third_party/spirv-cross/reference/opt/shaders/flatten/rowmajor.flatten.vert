#version 310 es

uniform vec4 UBO[12];
layout(location = 0) in vec4 aVertex;

void main()
{
    gl_Position = (mat4(UBO[0], UBO[1], UBO[2], UBO[3]) * aVertex) + (aVertex * mat4(UBO[4], UBO[5], UBO[6], UBO[7]));
}

