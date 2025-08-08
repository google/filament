#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 color [[color(0)]];
    float gl_FragDepth [[depth(any)]];
};

fragment main0_out main0(texture2d<float> inputDepth [[texture(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out.color = inputDepth.read(uint2(gl_FragCoord.xy));
    out.gl_FragDepth = 1.0;
    return out;
}

