#version 450 core

#extension GL_EXT_shader_16bit_storage : enable

struct S
{
    int16_t  x;
    i16vec2    y;
    i16vec3    z;
};

layout(column_major, std140) uniform B1
{
    int16_t  a;
    i16vec2    b;
    i16vec3    c;
    int16_t  d[2];
    S          g;
    S          h[2];
    int        j;
} b1;

layout(row_major, std430) buffer B2
{
    int16_t  o;
    i16vec2    p;
    i16vec3    q;
    int16_t  r[2];
    S          u;
    S          v[2];
    i16vec2    x[100];
    int16_t  w[];
} b2;

layout(row_major, std140) uniform B5
{
    int16_t  o;
    i16vec2    p;
    i16vec3    q;
    int16_t  r[2];
    S          u;
    S          v[2];
    i16vec2    x[100];
    int16_t  w[100];
} b5;

struct S2 {
    mat4x4 x;
    int16_t y;
    int z;
};

struct S3 {
    S2 x;
};

layout(row_major, std430) buffer B3
{
    S2 x;
} b3;

layout(column_major, std430) buffer B4
{
    S2 x;
    S3 y;
} b4;

void main()
{
    b2.o = b1.a;
    b2.p = i16vec2(ivec3(b2.q).xy);
    b2.p = i16vec2(ivec3(b5.q).xy);
    b2.r[0] = b2.r[0];
    b2.r[1] = b5.r[1];
    b2.p = b2.p;
    int x0 = int(b1.a);
    ivec4 x1 = ivec4(b1.a, b2.p, 1);
    b4.x.x = b3.x.x;
    b2.o = int16_t(ivec2(b2.p).x);
    b2.p = b2.v[1].y;
    ivec3 v3 = ivec3(b2.w[b1.j], b2.w[b1.j+1], b2.w[b1.j+2]);
    ivec3 u3 = ivec3(b5.w[b1.j], b5.w[b1.j+1], b5.w[b1.j+2]);
    b2.x[0] = b2.x[0];
    b2.x[1] = b5.x[1];
    b2.p.x = b1.a;
    b2.o = b2.p.x;
    b2.p = i16vec2(ivec2(1, 2));
    b2.o = int16_t(3);
}

