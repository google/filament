#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vColor [[user(locn0)]];
    float2 vTex_0 [[user(locn1)]];
    float2 vTex_1 [[user(locn2)]];
    float2 vTex_2 [[user(locn3)]];
    float2 vTex_3 [[user(locn4)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant uint* spvViewMask [[buffer(24)]], texture2d<float> uTex [[texture(0)]], sampler uTexSmplr [[sampler(0)]], uint gl_ViewIndex [[render_target_array_index]])
{
    main0_out out = {};
    float2 vTex[4] = {};
    vTex[0] = in.vTex_0;
    vTex[1] = in.vTex_1;
    vTex[2] = in.vTex_2;
    vTex[3] = in.vTex_3;
    gl_ViewIndex += spvViewMask[0];
    out.FragColor = in.vColor * uTex.sample(uTexSmplr, vTex[int(gl_ViewIndex)]);
    return out;
}

