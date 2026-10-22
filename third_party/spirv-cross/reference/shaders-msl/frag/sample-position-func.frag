#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 getColor(thread const int& i, thread float2& gl_SamplePosition)
{
    return float4(gl_SamplePosition, float(i), 1.0);
}

fragment main0_out main0(main0_in in [[stage_in]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    float2 gl_SamplePosition = get_sample_position(gl_SampleID);
    int param = in.index;
    out.FragColor = getColor(param, gl_SamplePosition);
    return out;
}

