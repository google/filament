#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = float3(as_type<float>(0x7f800000u), as_type<float>(0xff800000u), as_type<float>(0x7fc00000u));
    return out;
}

