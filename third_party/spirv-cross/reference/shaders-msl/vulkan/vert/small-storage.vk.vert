#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct block
{
    short2 a;
    ushort2 b;
    char2 c;
    uchar2 d;
    half2 e;
};

struct storage
{
    short3 f;
    ushort3 g;
    char3 h;
    uchar3 i;
    half3 j;
};

struct main0_out
{
    short4 p [[user(locn0)]];
    ushort4 q [[user(locn1)]];
    half4 r [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    short foo [[attribute(0)]];
    ushort bar [[attribute(1)]];
    half baz [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant block& _26 [[buffer(0)]], const device storage& _53 [[buffer(1)]])
{
    main0_out out = {};
    out.p = short4((int4(int(in.foo)) + int4(int2(_26.a), int2(_26.c))) - int4(int3(_53.f) / int3(_53.h), 1));
    out.q = ushort4((uint4(uint(in.bar)) + uint4(uint2(_26.b), uint2(_26.d))) - uint4(uint3(_53.g) / uint3(_53.i), 1u));
    out.r = half4((float4(float(in.baz)) + float4(float2(_26.e), 0.0, 1.0)) - float4(float3(_53.j), 1.0));
    out.gl_Position = float4(0.0, 0.0, 0.0, 1.0);
    return out;
}

