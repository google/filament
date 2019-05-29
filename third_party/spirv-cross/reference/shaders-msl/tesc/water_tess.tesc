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
    float2 vPatchPosBase [[attribute(0)]];
};

bool frustum_cull(thread const float2& p0, constant UBO& v_41)
{
    float2 min_xz = (p0 - float2(10.0)) * v_41.uScale.xy;
    float2 max_xz = ((p0 + v_41.uPatchSize) + float2(10.0)) * v_41.uScale.xy;
    float3 bb_min = float3(min_xz.x, -10.0, min_xz.y);
    float3 bb_max = float3(max_xz.x, 10.0, max_xz.y);
    float3 center = (bb_min + bb_max) * 0.5;
    float radius = 0.5 * length(bb_max - bb_min);
    float3 f0 = float3(dot(v_41.uFrustum[0], float4(center, 1.0)), dot(v_41.uFrustum[1], float4(center, 1.0)), dot(v_41.uFrustum[2], float4(center, 1.0)));
    float3 f1 = float3(dot(v_41.uFrustum[3], float4(center, 1.0)), dot(v_41.uFrustum[4], float4(center, 1.0)), dot(v_41.uFrustum[5], float4(center, 1.0)));
    float3 _199 = f0;
    float _200 = radius;
    bool _205 = any(_199 <= float3(-_200));
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

float lod_factor(thread const float2& pos_, constant UBO& v_41)
{
    float2 pos = pos_ * v_41.uScale.xy;
    float3 dist_to_cam = v_41.uCamPos - float3(pos.x, 0.0, pos.y);
    float level = log2((length(dist_to_cam) + 9.9999997473787516355514526367188e-05) * v_41.uDistanceMod);
    return fast::clamp(level, 0.0, v_41.uMaxTessLevel.x);
}

float4 tess_level(thread const float4& lod, constant UBO& v_41)
{
    return exp2(-lod) * v_41.uMaxTessLevel.y;
}

float tess_level(thread const float& lod, constant UBO& v_41)
{
    return v_41.uMaxTessLevel.y * exp2(-lod);
}

void compute_tess_levels(thread const float2& p0, constant UBO& v_41, device float2& vOutPatchPosBase, device float4& vPatchLods, device half (&gl_TessLevelOuter)[4], device half (&gl_TessLevelInner)[2])
{
    vOutPatchPosBase = p0;
    float2 param = p0 + (float2(-0.5) * v_41.uPatchSize);
    float l00 = lod_factor(param, v_41);
    float2 param_1 = p0 + (float2(0.5, -0.5) * v_41.uPatchSize);
    float l10 = lod_factor(param_1, v_41);
    float2 param_2 = p0 + (float2(1.5, -0.5) * v_41.uPatchSize);
    float l20 = lod_factor(param_2, v_41);
    float2 param_3 = p0 + (float2(-0.5, 0.5) * v_41.uPatchSize);
    float l01 = lod_factor(param_3, v_41);
    float2 param_4 = p0 + (float2(0.5) * v_41.uPatchSize);
    float l11 = lod_factor(param_4, v_41);
    float2 param_5 = p0 + (float2(1.5, 0.5) * v_41.uPatchSize);
    float l21 = lod_factor(param_5, v_41);
    float2 param_6 = p0 + (float2(-0.5, 1.5) * v_41.uPatchSize);
    float l02 = lod_factor(param_6, v_41);
    float2 param_7 = p0 + (float2(0.5, 1.5) * v_41.uPatchSize);
    float l12 = lod_factor(param_7, v_41);
    float2 param_8 = p0 + (float2(1.5) * v_41.uPatchSize);
    float l22 = lod_factor(param_8, v_41);
    float4 lods = float4(dot(float4(l01, l11, l02, l12), float4(0.25)), dot(float4(l00, l10, l01, l11), float4(0.25)), dot(float4(l10, l20, l11, l21), float4(0.25)), dot(float4(l11, l21, l12, l22), float4(0.25)));
    vPatchLods = lods;
    float4 outer_lods = fast::min(lods, lods.yzwx);
    float4 param_9 = outer_lods;
    float4 levels = tess_level(param_9, v_41);
    gl_TessLevelOuter[0] = half(levels.x);
    gl_TessLevelOuter[1] = half(levels.y);
    gl_TessLevelOuter[2] = half(levels.z);
    gl_TessLevelOuter[3] = half(levels.w);
    float min_lod = fast::min(fast::min(lods.x, lods.y), fast::min(lods.z, lods.w));
    float param_10 = fast::min(min_lod, l11);
    float inner = tess_level(param_10, v_41);
    gl_TessLevelInner[0] = half(inner);
    gl_TessLevelInner[1] = half(inner);
}

kernel void main0(main0_in in [[stage_in]], constant UBO& v_41 [[buffer(0)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 1)
        return;
    float2 p0 = gl_in[0].vPatchPosBase;
    float2 param = p0;
    if (!frustum_cull(param, v_41))
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
        compute_tess_levels(param_1, v_41, patchOut.vOutPatchPosBase, patchOut.vPatchLods, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor, spvTessLevel[gl_PrimitiveID].insideTessellationFactor);
    }
}

