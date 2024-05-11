#version 450

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

struct t21
{
    vec4 m0;
    vec4 m1;
};

layout(location = 0) in vec4 v17;
layout(location = 0) out t24
{
    t21 m0[3];
} v26;


void main()
{
    gl_Position = v17;
    v26.m0[1].m1 = vec4(-4.0, -9.0, 3.0, 7.0);
}
