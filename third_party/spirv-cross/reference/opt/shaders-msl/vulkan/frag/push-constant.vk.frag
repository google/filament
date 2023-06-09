#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PushConstants
{
    float4 value0;
    float4 value1;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant PushConstants& push [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = (in.vColor + push.value0) + push.value1;
    return out;
}

