#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4 uScale;
    float3 uCamPos;
    float2 uPatchSize;
    float2 uMaxTessLevel;
    float uDistanceMod;
    float4 uFrustum[6];
};

struct main0_patchOut
{
    float2 vOutPatchPosBase;
    float4 vPatchLods;
};

struct main0_in
{
    float3 vPatchPosBase;
    ushort2 m_430;
};

static inline __attribute__((always_inline))
bool frustum_cull(thread const float2& p0, constant UBO& _41)
{
    float2 min_xz = (p0 - float2(10.0)) * _41.uScale.xy;
    float2 max_xz = ((p0 + _41.uPatchSize) + float2(10.0)) * _41.uScale.xy;
    float3 bb_min = float3(min_xz.x, -10.0, min_xz.y);
    float3 bb_max = float3(max_xz.x, 10.0, max_xz.y);
    float3 center = (bb_min + bb_max) * 0.5;
    float radius = 0.5 * length(bb_max - bb_min);
    float3 f0 = float3(dot(_41.uFrustum[0], float4(center, 1.0)), dot(_41.uFrustum[1], float4(center, 1.0)), dot(_41.uFrustum[2], float4(center, 1.0)));
    float3 f1 = float3(dot(_41.uFrustum[3], float4(center, 1.0)), dot(_41.uFrustum[4], float4(center, 1.0)), dot(_41.uFrustum[5], float4(center, 1.0)));
    bool _205 = any(f0 <= float3(-radius));
    bool _215;
    if (!_205)
    {
        _215 = any(f1 <= float3(-radius));
    }
    else
    {
        _215 = _205;
    }
    return !_215;
}

static inline __attribute__((always_inline))
float lod_factor(thread const float2& pos_, constant UBO& _41)
{
    float2 pos = pos_ * _41.uScale.xy;
    float3 dist_to_cam = _41.uCamPos - float3(pos.x, 0.0, pos.y);
    float level0 = log2((length(dist_to_cam) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod);
    return fast::clamp(level0, 0.0, _41.uMaxTessLevel.x);
}

static inline __attribute__((always_inline))
float4 tess_level(thread const float4& lod, constant UBO& _41)
{
    return exp2(-lod) * _41.uMaxTessLevel.y;
}

static inline __attribute__((always_inline))
float tess_level(thread const float& lod, constant UBO& _41)
{
    return _41.uMaxTessLevel.y * exp2(-lod);
}

static inline __attribute__((always_inline))
void compute_tess_levels(thread const float2& p0, constant UBO& _41, device float2& vOutPatchPosBase, device float4& vPatchLods, device half (&gl_TessLevelOuter)[4], device half (&gl_TessLevelInner)[2])
{
    vOutPatchPosBase = p0;
    float2 param = p0 + (float2(-0.5) * _41.uPatchSize);
    float l00 = lod_factor(param, _41);
    float2 param_1 = p0 + (float2(0.5, -0.5) * _41.uPatchSize);
    float l10 = lod_factor(param_1, _41);
    float2 param_2 = p0 + (float2(1.5, -0.5) * _41.uPatchSize);
    float l20 = lod_factor(param_2, _41);
    float2 param_3 = p0 + (float2(-0.5, 0.5) * _41.uPatchSize);
    float l01 = lod_factor(param_3, _41);
    float2 param_4 = p0 + (float2(0.5) * _41.uPatchSize);
    float l11 = lod_factor(param_4, _41);
    float2 param_5 = p0 + (float2(1.5, 0.5) * _41.uPatchSize);
    float l21 = lod_factor(param_5, _41);
    float2 param_6 = p0 + (float2(-0.5, 1.5) * _41.uPatchSize);
    float l02 = lod_factor(param_6, _41);
    float2 param_7 = p0 + (float2(0.5, 1.5) * _41.uPatchSize);
    float l12 = lod_factor(param_7, _41);
    float2 param_8 = p0 + (float2(1.5) * _41.uPatchSize);
    float l22 = lod_factor(param_8, _41);
    float4 lods = float4(dot(float4(l01, l11, l02, l12), float4(0.25)), dot(float4(l00, l10, l01, l11), float4(0.25)), dot(float4(l10, l20, l11, l21), float4(0.25)), dot(float4(l11, l21, l12, l22), float4(0.25)));
    vPatchLods = lods;
    float4 outer_lods = fast::min(lods, lods.yzwx);
    float4 param_9 = outer_lods;
    float4 levels = tess_level(param_9, _41);
    gl_TessLevelOuter[0] = half(levels.x);
    gl_TessLevelOuter[1] = half(levels.y);
    gl_TessLevelOuter[2] = half(levels.z);
    gl_TessLevelOuter[3] = half(levels.w);
    float min_lod = fast::min(fast::min(lods.x, lods.y), fast::min(lods.z, lods.w));
    float param_10 = fast::min(min_lod, l11);
    float inner = tess_level(param_10, _41);
    gl_TessLevelInner[0] = half(inner);
    gl_TessLevelInner[1] = half(inner);
}

kernel void main0(constant UBO& _41 [[buffer(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 1];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1] - 1);
    float2 p0 = gl_in[0].vPatchPosBase.xy;
    float2 param = p0;
    if (!frustum_cull(param, _41))
    {
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(-1.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(-1.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(-1.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(-1.0);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(-1.0);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(-1.0);
    }
    else
    {
        float2 param_1 = p0;
        compute_tess_levels(param_1, _41, patchOut.vOutPatchPosBase, patchOut.vPatchLods, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor, spvTessLevel[gl_PrimitiveID].insideTessellationFactor);
    }
}

