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

fragment main0_out main0(main0_in in [[stage_in]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    float2 gl_SamplePosition = get_sample_position(gl_SampleID);
    out.FragColor = float4(gl_SamplePosition, float(in.index), 1.0);
    return out;
}

