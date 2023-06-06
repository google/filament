#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(1)]];
};

fragment main0_out main0(float4 uInput [[color(1)]])
{
    main0_out out = {};
    out.FragColor = uInput;
    return out;
}

