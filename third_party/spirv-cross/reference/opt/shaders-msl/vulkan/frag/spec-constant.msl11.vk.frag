#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#ifndef SPIRV_CROSS_CONSTANT_ID_1
#define SPIRV_CROSS_CONSTANT_ID_1 1.0
#endif
constant float a = SPIRV_CROSS_CONSTANT_ID_1;
#ifndef SPIRV_CROSS_CONSTANT_ID_2
#define SPIRV_CROSS_CONSTANT_ID_2 2.0
#endif
constant float b = SPIRV_CROSS_CONSTANT_ID_2;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = float4(a + b);
    return out;
}

