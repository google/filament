#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o1 [[color(1)]];
    float4 o3 [[color(3)]];
    float4 o6 [[color(6)]];
    float4 o7 [[color(7)]];
};

fragment main0_out main0()
{
    float4 o0;
    float4 o2;
    float4 o4;
    float4 o5;
    float gl_FragDepth;
    int gl_FragStencilRefARB;
    main0_out out = {};
    o0 = float4(0.0, 0.0, 0.0, 1.0);
    out.o1 = float4(1.0, 0.0, 0.0, 1.0);
    o2 = float4(0.0, 1.0, 0.0, 1.0);
    out.o3 = float4(0.0, 0.0, 1.0, 1.0);
    o4 = float4(1.0, 0.0, 1.0, 0.5);
    o5 = float4(0.25);
    out.o6 = float4(0.75);
    out.o7 = float4(1.0);
    gl_FragDepth = 0.89999997615814208984375;
    gl_FragStencilRefARB = uint(127);
    return out;
}

