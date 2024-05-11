#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 color_out [[color(0)]];
    float gl_FragDepth [[depth(less)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.color_out = float4(1.0, 0.0, 0.0, 1.0);
    out.gl_FragDepth = 0.699999988079071044921875;
    return out;
}

