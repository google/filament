#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position, invariant]];
};

static inline __attribute__((always_inline))
float4 _main()
{
    return float4(1.0);
}

vertex main0_out main0()
{
    main0_out out = {};
    float4 _14 = _main();
    out.gl_Position = _14;
    return out;
}

