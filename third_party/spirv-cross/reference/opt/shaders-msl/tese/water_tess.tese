#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 uMVP;
    float4 uScale;
    float2 uInvScale;
    float3 uCamPos;
    float2 uPatchSize;
    float2 uInvHeightmapSize;
};

struct main0_out
{
    float3 vWorld [[user(locn0)]];
    float4 vGradNormalTex [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float2 vOutPatchPosBase [[attribute(0)]];
    float4 vPatchLods [[attribute(1)]];
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant UBO& _31 [[buffer(0)]], texture2d<float> uHeightmapDisplacement [[texture(0)]], sampler uHeightmapDisplacementSmplr [[sampler(0)]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    float2 _204 = patchIn.vOutPatchPosBase + (float3(gl_TessCoord, 0).xy * _31.uPatchSize);
    float2 _219 = mix(patchIn.vPatchLods.yx, patchIn.vPatchLods.zw, float2(float3(gl_TessCoord, 0).x));
    float _226 = mix(_219.x, _219.y, float3(gl_TessCoord, 0).y);
    float _228 = floor(_226);
    float2 _127 = _204 * _31.uInvHeightmapSize;
    float2 _143 = _31.uInvHeightmapSize * exp2(_228);
    out.vGradNormalTex = float4(_127 + (_31.uInvHeightmapSize * 0.5), _127 * _31.uScale.zw);
    float3 _260 = mix(uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (_127 + (_143 * 0.5)), level(_228)).xyz, uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (_127 + (_143 * 1.0)), level(_228 + 1.0)).xyz, float3(_226 - _228));
    float2 _173 = (_204 * _31.uScale.xy) + _260.yz;
    out.vWorld = float3(_173.x, _260.x, _173.y);
    out.gl_Position = _31.uMVP * float4(out.vWorld, 1.0);
    return out;
}

