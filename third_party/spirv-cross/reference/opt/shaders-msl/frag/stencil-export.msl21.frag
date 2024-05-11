#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 MRT0 [[color(0)]];
    float4 MRT1 [[color(1)]];
    uint gl_FragStencilRefARB [[stencil]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.MRT0 = float4(1.0);
    out.MRT1 = float4(1.0, 0.0, 1.0, 1.0);
    out.gl_FragStencilRefARB = uint(100);
    out.gl_FragStencilRefARB = uint(int(out.gl_FragStencilRefARB) + 10);
    return out;
}

