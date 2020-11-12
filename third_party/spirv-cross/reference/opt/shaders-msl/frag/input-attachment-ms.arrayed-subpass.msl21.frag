#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d_ms_array<float> uSubpass0 [[texture(0)]], texture2d_ms_array<float> uSubpass1 [[texture(1)]], uint gl_SampleID [[sample_id]], float4 gl_FragCoord [[position]], uint gl_Layer [[render_target_array_index]])
{
    main0_out out = {};
    out.FragColor = (uSubpass0.read(uint2(gl_FragCoord.xy), gl_Layer, 1) + uSubpass1.read(uint2(gl_FragCoord.xy), gl_Layer, 2)) + uSubpass0.read(uint2(gl_FragCoord.xy), gl_Layer, gl_SampleID);
    return out;
}

