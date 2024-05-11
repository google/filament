#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Returns 2D texture coords corresponding to 1D texel buffer coords
static inline __attribute__((always_inline))
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(texture2d<float> uSamp [[texture(0)]], texture2d<float> uSampo [[texture(1)]])
{
    main0_out out = {};
    out.gl_Position = uSamp.read(spvTexelBufferCoord(10)) + uSampo.read(spvTexelBufferCoord(100));
    return out;
}

