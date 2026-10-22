#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    packed_float3 color;
    float v;
};

struct main0_out
{
    float3 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vIn [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO& _15 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = cross(in.vIn, float3(_15.color) - in.vIn);
    return out;
}

