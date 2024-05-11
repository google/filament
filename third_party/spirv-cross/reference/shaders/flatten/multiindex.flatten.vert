#version 310 es

uniform vec4 UBO[15];
layout(location = 0) in ivec2 aIndex;

void main()
{
    gl_Position = UBO[aIndex.x * 5 + aIndex.y * 1 + 0];
}

