#version 450

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

struct s0
{
    mediump mat2x3 m0;
    ivec4 m1;
    mat4 m2;
    uvec2 m3;
};

struct s1
{
    mediump mat3x4 m0;
    mediump int m1;
    uvec3 m2;
    s0 m3;
};

layout(set = 0, binding = 0, std140) uniform data_u_t
{
    layout(row_major, offset = 368) mediump mat2x3 m0;
    layout(offset = 0) vec2 m1[5];
    layout(row_major, offset = 128) s1 m2;
    layout(row_major, offset = 80) mediump mat4x2 m3;
    layout(offset = 112) ivec4 m4;
} data_u;

layout(location = 0) in vec4 vtx_posn;
layout(location = 0) out mediump float foo;

void main()
{
    gl_Position = vtx_posn;
    vec2 a = data_u.m1[3];
    ivec4 b = data_u.m4;
    mat2x3 c = data_u.m0;
    mat3x4 d = data_u.m2.m0;
    mat4 e = data_u.m2.m3.m2;
    foo = (a.y + b.z) * c[1][2] * d[2][3] * e[3][3];
}


