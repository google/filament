#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct data_u_t
{
    int4 m1[3];
    uint m3;
    uint3 m2;
    int4 m0[8];
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
    out.foo = float((uint3(data_u.m1[1].xyz) + data_u.m2).y * uint(data_u.m0[4].x));
    return out;
}

