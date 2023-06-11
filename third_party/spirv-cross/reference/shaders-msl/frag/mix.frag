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
    bool4 l = bool4(false, true, false, false);
    out.FragColor = select(in.vIn0, in.vIn1, l);
    bool f = true;
    out.FragColor = float4(f ? in.vIn3 : in.vIn2);
    out.FragColor = select(in.vIn1, in.vIn0, bool4(f));
    out.FragColor = float4(f ? in.vIn2 : in.vIn3);
    return out;
}

