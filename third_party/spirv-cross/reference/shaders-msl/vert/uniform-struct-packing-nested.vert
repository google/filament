#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef packed_float4 packed_rm_float4x4[4];

struct s0
{
    float3x4 m0;
    packed_int4 m1;
    packed_rm_float4x4 m2;
    packed_uint2 m3;
};

struct s1
{
    float4x4 m0;
    int m1;
    char _m2_pad[12];
    packed_uint3 m2;
    s0 m3;
};

struct data_u_t
{
    float4 m1[5];
    float2x4 m3;
    int4 m4;
    s1 m2;
    float3x4 m0;
};

struct main0_out
{
    float foo [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vtx_posn [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant data_u_t& data_u [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = in.vtx_posn;
    float2 a = data_u.m1[3].xy;
    int4 b = data_u.m4;
    float2x3 c = transpose(float3x2(data_u.m0[0].xy, data_u.m0[1].xy, data_u.m0[2].xy));
    float3x4 d = transpose(float4x3(data_u.m2.m0[0].xyz, data_u.m2.m0[1].xyz, data_u.m2.m0[2].xyz, data_u.m2.m0[3].xyz));
    float4x4 e = transpose(float4x4(float4(data_u.m2.m3.m2[0]), float4(data_u.m2.m3.m2[1]), float4(data_u.m2.m3.m2[2]), float4(data_u.m2.m3.m2[3])));
    out.foo = (((a.y + float(b.z)) * c[1].z) * d[2].w) * e[3].w;
    return out;
}

