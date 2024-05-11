#version 450

layout(location = 0) out VertexData
{
    flat float f;
    centroid vec4 g;
    flat int h;
    float i;
} vout;

layout(location = 4) flat out float f;
layout(location = 5) centroid out vec4 g;
layout(location = 6) flat out int h;
layout(location = 7) out float i;

void main()
{
    vout.f = 10.0;
    vout.g = vec4(20.0);
    vout.h = 20;
    vout.i = 30.0;
    f = 10.0;
    g = vec4(20.0);
    h = 20;
    i = 30.0;
}

