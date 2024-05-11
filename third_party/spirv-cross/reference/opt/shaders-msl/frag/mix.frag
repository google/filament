#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vIn0 [[user(locn0)]];
    float4 vIn1 [[user(locn1)]];
    float vIn2 [[user(locn2)]];
    float vIn3 [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = float4(in.vIn0.x, in.vIn1.y, in.vIn0.z, in.vIn0.w);
    out.FragColor = float4(in.vIn3);
    out.FragColor = in.vIn0.xyzw;
    out.FragColor = float4(in.vIn2);
    return out;
}

