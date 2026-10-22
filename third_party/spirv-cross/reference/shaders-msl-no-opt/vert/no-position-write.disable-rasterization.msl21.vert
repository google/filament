#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    int o_value [[user(locn0)]];
    float4 gl_Position [[position, invariant]];
};

vertex void main0()
{
    main0_out out = {};
    out.o_value = 10;
    out.gl_Position = float4(10.0);
}

