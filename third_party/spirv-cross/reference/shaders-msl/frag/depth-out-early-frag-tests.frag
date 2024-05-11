#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 color_out [[color(0)]];
};

[[ early_fragment_tests ]] fragment main0_out main0()
{
    float gl_FragDepth;
    main0_out out = {};
    out.color_out = float4(1.0, 0.0, 0.0, 1.0);
    gl_FragDepth = 0.699999988079071044921875;
    return out;
}

