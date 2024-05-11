#version 450
layout(triangles) in;
layout(max_vertices = 4, triangle_strip) out;

layout(binding = 0, std140) uniform VertexInput
{
    vec4 a;
} VertexInput_1;

layout(binding = 0, std430) buffer VertexInput
{
    vec4 b;
} VertexInput_2;

layout(location = 0) out VertexInput
{
    vec4 vColor;
} VertexInput_3;

layout(location = 0) in VertexInput
{
    vec4 vColor;
} vin[3];


void main()
{
    vec4 VertexInput_4 = vec4(1.0);
    gl_Position = (VertexInput_4 + VertexInput_1.a) + VertexInput_2.b;
    VertexInput_3.vColor = vin[0].vColor;
    EmitVertex();
}

