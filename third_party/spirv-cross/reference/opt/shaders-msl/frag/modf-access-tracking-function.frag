#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 vo0 [[color(0)]];
    float4 vo1 [[color(1)]];
};

struct main0_in
{
    float4 v [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 _25 = modf(in.v, out.vo1);
    out.vo0 = _25;
    return out;
}

