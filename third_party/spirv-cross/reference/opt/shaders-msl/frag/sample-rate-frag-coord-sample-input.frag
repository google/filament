#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float foo [[user(locn0), sample_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d_array<float> tex [[texture(0)]], sampler texSmplr [[sampler(0)]], float4 gl_FragCoord [[position]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    gl_FragCoord.xy += get_sample_position(gl_SampleID) - 0.5;
    float3 _26 = float3(gl_FragCoord.xy, in.foo);
    out.FragColor = tex.sample(texSmplr, _26.xy, uint(rint(_26.z)));
    return out;
}

