#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d_array<float> tex [[texture(0)]], sampler texSmplr [[sampler(0)]], float4 gl_FragCoord [[position]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    gl_FragCoord.xy += get_sample_position(gl_SampleID) - 0.5;
    float3 _28 = float3(gl_FragCoord.xy, float(gl_SampleID));
    out.FragColor = tex.sample(texSmplr, _28.xy, uint(round(_28.z)));
    return out;
}

