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
    float m0;
    vec4 m1;
};

layout(location = 0) in vec4 v17;
layout(location = 0) out t21 v25[3];

void main()
{
    gl_Position = v17;
    v25[2].m1 = vec4(-4.0, -9.0, 3.0, 7.0);
}
