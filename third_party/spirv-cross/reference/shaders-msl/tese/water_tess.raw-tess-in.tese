#pragma clang diagnostic ignored "-Wmissing-prototypes"

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
    float2 vOutPatchPosBase;
    float4 vPatchLods;
};

static inline __attribute__((always_inline))
float2 lerp_vertex(thread const float2& tess_coord, const device float2& vOutPatchPosBase, constant UBO& _31)
{
    return vOutPatchPosBase + (tess_coord * _31.uPatchSize);
}

static inline __attribute__((always_inline))
float2 lod_factor(thread const float2& tess_coord, const device float4& vPatchLods)
{
    float2 x = mix(vPatchLods.yx, vPatchLods.zw, float2(tess_coord.x));
    float level0 = mix(x.x, x.y, tess_coord.y);
    float floor_level = floor(level0);
    float fract_level = level0 - floor_level;
    return float2(floor_level, fract_level);
}

static inline __attribute__((always_inline))
float3 sample_height_displacement(thread const float2& uv, thread const float2& off, thread const float2& lod, texture2d<float> uHeightmapDisplacement, sampler uHeightmapDisplacementSmplr)
{
    return mix(uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (uv + (off * 0.5)), level(lod.x)).xyz, uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (uv + (off * 1.0)), level(lod.x + 1.0)).xyz, float3(lod.y));
}

[[ patch(quad, 0) ]] vertex main0_out main0(constant UBO& _31 [[buffer(0)]], texture2d<float> uHeightmapDisplacement [[texture(0)]], sampler uHeightmapDisplacementSmplr [[sampler(0)]], float2 gl_TessCoordIn [[position_in_patch]], uint gl_PrimitiveID [[patch_id]], const device main0_patchIn* spvPatchIn [[buffer(20)]])
{
    main0_out out = {};
    const device main0_patchIn& patchIn = spvPatchIn[gl_PrimitiveID];
    float3 gl_TessCoord = float3(gl_TessCoordIn.x, gl_TessCoordIn.y, 0.0);
    float2 tess_coord = gl_TessCoord.xy;
    float2 param = tess_coord;
    float2 pos = lerp_vertex(param, patchIn.vOutPatchPosBase, _31);
    float2 param_1 = tess_coord;
    float2 lod = lod_factor(param_1, patchIn.vPatchLods);
    float2 tex = pos * _31.uInvHeightmapSize;
    pos *= _31.uScale.xy;
    float delta_mod = exp2(lod.x);
    float2 off = _31.uInvHeightmapSize * delta_mod;
    out.vGradNormalTex = float4(tex + (_31.uInvHeightmapSize * 0.5), tex * _31.uScale.zw);
    float2 param_2 = tex;
    float2 param_3 = off;
    float2 param_4 = lod;
    float3 height_displacement = sample_height_displacement(param_2, param_3, param_4, uHeightmapDisplacement, uHeightmapDisplacementSmplr);
    pos += height_displacement.yz;
    out.vWorld = float3(pos.x, height_displacement.x, pos.y);
    out.gl_Position = _31.uMVP * float4(out.vWorld, 1.0);
    return out;
}

