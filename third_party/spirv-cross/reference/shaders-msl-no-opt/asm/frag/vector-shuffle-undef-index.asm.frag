#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 undef = {};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vFloat [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = float4(undef.x, in.vFloat.y, 0.0, in.vFloat.w) + float4(in.vFloat.z, in.vFloat.y, 0.0, in.vFloat.w);
    return out;
}

