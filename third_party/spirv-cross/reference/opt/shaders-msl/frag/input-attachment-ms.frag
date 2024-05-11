#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d_ms<float> uSubpass0 [[texture(0)]], texture2d_ms<float> uSubpass1 [[texture(1)]], uint gl_SampleID [[sample_id]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    gl_FragCoord.xy += get_sample_position(gl_SampleID) - 0.5;
    out.FragColor = (uSubpass0.read(uint2(gl_FragCoord.xy), 1) + uSubpass1.read(uint2(gl_FragCoord.xy), 2)) + uSubpass0.read(uint2(gl_FragCoord.xy), gl_SampleID);
    return out;
}

