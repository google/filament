#version 450
#extension GL_NV_geometry_shader_passthrough : require
layout(triangles) in;

layout(passthrough) in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

layout(passthrough, location = 0) in VertexBlock
{
    int a;
    int b;
} v1[3];

layout(location = 2) in VertexBlock2
{
    int a;
    layout(passthrough) int b;
} v2[3];


void main()
{
    gl_Layer = (gl_InvocationID + v1[0].a) + v2[1].b;
}

