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
    float2 vOutPatchPosBase [[attribute(0)]];
    float4 vPatchLods [[attribute(1)]];
};

static inline __attribute__((always_inline))
float2 lerp_vertex(thread const float2& tess_coord, thread float2& vOutPatchPosBase, constant UBO& v_31)
{
    return vOutPatchPosBase + (tess_coord * v_31.uPatchSize);
}

static inline __attribute__((always_inline))
float2 lod_factor(thread const float2& tess_coord, thread float4& vPatchLods)
{
    float2 x = mix(vPatchLods.yx, vPatchLods.zw, float2(tess_coord.x));
    float level = mix(x.x, x.y, tess_coord.y);
    float floor_level = floor(level);
    float fract_level = level - floor_level;
    return float2(floor_level, fract_level);
}

static inline __attribute__((always_inline))
float3 sample_height_displacement(thread const float2& uv, thread const float2& off, thread const float2& lod, thread texture2d<float> uHeightmapDisplacement, thread const sampler uHeightmapDisplacementSmplr)
{
    return mix(uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (uv + (off * 0.5)), level(lod.x)).xyz, uHeightmapDisplacement.sample(uHeightmapDisplacementSmplr, (uv + (off * 1.0)), level(lod.x + 1.0)).xyz, float3(lod.y));
}

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant UBO& v_31 [[buffer(0)]], texture2d<float> uHeightmapDisplacement [[texture(0)]], sampler uHeightmapDisplacementSmplr [[sampler(0)]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    float2 tess_coord = float3(gl_TessCoord, 0).xy;
    float2 param = tess_coord;
    float2 pos = lerp_vertex(param, patchIn.vOutPatchPosBase, v_31);
    float2 param_1 = tess_coord;
    float2 lod = lod_factor(param_1, patchIn.vPatchLods);
    float2 tex = pos * v_31.uInvHeightmapSize;
    pos *= v_31.uScale.xy;
    float delta_mod = exp2(lod.x);
    float2 off = v_31.uInvHeightmapSize * delta_mod;
    out.vGradNormalTex = float4(tex + (v_31.uInvHeightmapSize * 0.5), tex * v_31.uScale.zw);
    float2 param_2 = tex;
    float2 param_3 = off;
    float2 param_4 = lod;
    float3 height_displacement = sample_height_displacement(param_2, param_3, param_4, uHeightmapDisplacement, uHeightmapDisplacementSmplr);
    pos += height_displacement.yz;
    out.vWorld = float3(pos.x, height_displacement.x, pos.y);
    out.gl_Position = v_31.uMVP * float4(out.vWorld, 1.0);
    return out;
}

