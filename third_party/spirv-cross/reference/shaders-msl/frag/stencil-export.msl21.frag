#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 MRT0 [[color(0)]];
    float4 MRT1 [[color(1)]];
    uint gl_FragStencilRefARB [[stencil]];
};

void update_stencil(thread uint& gl_FragStencilRefARB)
{
    gl_FragStencilRefARB = uint(int(gl_FragStencilRefARB) + 10);
}

fragment main0_out main0()
{
    main0_out out = {};
    out.MRT0 = float4(1.0);
    out.MRT1 = float4(1.0, 0.0, 1.0, 1.0);
    out.gl_FragStencilRefARB = uint(100);
    update_stencil(out.gl_FragStencilRefARB);
    return out;
}

