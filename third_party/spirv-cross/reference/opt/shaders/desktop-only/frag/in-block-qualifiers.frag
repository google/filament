#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in VertexData
{
    flat float f;
    centroid vec4 g;
    flat int h;
    float i;
} vin;

layout(location = 4) flat in float f;
layout(location = 5) centroid in vec4 g;
layout(location = 6) flat in int h;
layout(location = 7) sample in float i;

void main()
{
    FragColor = ((((((vec4(vin.f) + vin.g) + vec4(float(vin.h))) + vec4(vin.i)) + vec4(f)) + g) + vec4(float(h))) + vec4(i);
}

