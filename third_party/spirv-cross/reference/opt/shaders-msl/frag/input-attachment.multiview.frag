#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant uint* spvViewMask [[buffer(24)]], texture2d_array<float> uSubpass0 [[texture(0)]], texture2d_array<float> uSubpass1 [[texture(1)]], float4 gl_FragCoord [[position]], uint gl_ViewIndex [[render_target_array_index]])
{
    main0_out out = {};
    gl_ViewIndex += spvViewMask[0];
    out.FragColor = uSubpass0.read(uint2(gl_FragCoord.xy), gl_ViewIndex) + uSubpass1.read(uint2(gl_FragCoord.xy), gl_ViewIndex);
    return out;
}

