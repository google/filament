#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float4 samp3(float4 uS)
{
    return uS;
}

static inline __attribute__((always_inline))
float4 samp(float4 uSub)
{
    return uSub + samp3(uSub);
}

static inline __attribute__((always_inline))
float4 samp2(float4 uS)
{
    return uS + samp3(uS);
}

fragment main0_out main0(float4 uSub [[color(0)]])
{
    main0_out out = {};
    out.FragColor = samp(uSub) + samp2(uSub);
    return out;
}

