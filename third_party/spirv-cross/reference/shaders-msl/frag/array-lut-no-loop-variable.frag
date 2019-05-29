#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float _17[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    for (int i = 0; i < 4; i++, out.FragColor += float4(_17[i]))
    {
    }
    return out;
}

