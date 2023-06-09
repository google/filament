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
    out.foo = (((data_u.m1[3].y + float(data_u.m4.z)) * data_u.m0[2][1]) * data_u.m2.m0[3][2]) * data_u.m2.m3.m2[3][3];
    return out;
}

