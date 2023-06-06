#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 buf1 [[color(1)]];
    float4 buf3 [[color(3)]];
    float4 buf6 [[color(6)]];
    float4 buf7 [[color(7)]];
};

fragment main0_out main0()
{
    float4 buf0;
    float4 buf2;
    float4 buf4;
    float4 buf5;
    float gl_FragDepth;
    int gl_FragStencilRefARB;
    main0_out out = {};
    buf0 = float4(0.0, 0.0, 0.0, 1.0);
    out.buf1 = float4(1.0, 0.0, 0.0, 1.0);
    buf2 = float4(0.0, 1.0, 0.0, 1.0);
    out.buf3 = float4(0.0, 0.0, 1.0, 1.0);
    buf4 = float4(1.0, 0.0, 1.0, 0.5);
    buf5 = float4(0.25);
    out.buf6 = float4(0.75);
    out.buf7 = float4(1.0);
    gl_FragDepth = 0.89999997615814208984375;
    gl_FragStencilRefARB = uint(127);
    return out;
}

