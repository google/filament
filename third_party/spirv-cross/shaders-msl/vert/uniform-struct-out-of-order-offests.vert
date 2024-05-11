#version 450

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

layout(set = 0, binding = 0, std140) uniform data_u_t
{
    layout(offset = 80) mediump int m0[8];
    layout(offset = 0) mediump ivec4 m1[3];
    layout(offset = 64) uvec3 m2;
    layout(offset = 48) mediump uint m3;
} data_u;

layout(location = 0) in vec4 vtx_posn;
layout(location = 0) out mediump float foo;

void main()
{
    gl_Position = vtx_posn;
    ivec4 a = data_u.m1[1];
    uvec3 b = data_u.m2;
    int c = data_u.m0[4];
    foo = (a.xyz + b).y * c;
}


