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

kernel void main0(main0_in in [[stage_in]], constant UBO& _41 [[buffer(0)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 1)
        return;
    float2 _430 = (gl_in[0].vPatchPosBase - float2(10.0)) * _41.uScale.xy;
    float2 _440 = ((gl_in[0].vPatchPosBase + _41.uPatchSize) + float2(10.0)) * _41.uScale.xy;
    float3 _445 = float3(_430.x, -10.0, _430.y);
    float3 _450 = float3(_440.x, 10.0, _440.y);
    float4 _466 = float4((_445 + _450) * 0.5, 1.0);
    float3 _513 = float3(length(_450 - _445) * (-0.5));
    bool _515 = any(float3(dot(_41.uFrustum[0], _466), dot(_41.uFrustum[1], _466), dot(_41.uFrustum[2], _466)) <= _513);
    bool _525;
    if (!_515)
    {
        _525 = any(float3(dot(_41.uFrustum[3], _466), dot(_41.uFrustum[4], _466), dot(_41.uFrustum[5], _466)) <= _513);
    }
    else
    {
        _525 = _515;
    }
    if (!(!_525))
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
        patchOut.vOutPatchPosBase = gl_in[0].vPatchPosBase;
        float2 _678 = (gl_in[0].vPatchPosBase + (float2(-0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float2 _706 = (gl_in[0].vPatchPosBase + (float2(0.5, -0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _725 = fast::clamp(log2((length(_41.uCamPos - float3(_706.x, 0.0, _706.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _734 = (gl_in[0].vPatchPosBase + (float2(1.5, -0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float2 _762 = (gl_in[0].vPatchPosBase + (float2(-0.5, 0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _781 = fast::clamp(log2((length(_41.uCamPos - float3(_762.x, 0.0, _762.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _790 = (gl_in[0].vPatchPosBase + (float2(0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _809 = fast::clamp(log2((length(_41.uCamPos - float3(_790.x, 0.0, _790.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _818 = (gl_in[0].vPatchPosBase + (float2(1.5, 0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _837 = fast::clamp(log2((length(_41.uCamPos - float3(_818.x, 0.0, _818.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _846 = (gl_in[0].vPatchPosBase + (float2(-0.5, 1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float2 _874 = (gl_in[0].vPatchPosBase + (float2(0.5, 1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _893 = fast::clamp(log2((length(_41.uCamPos - float3(_874.x, 0.0, _874.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _902 = (gl_in[0].vPatchPosBase + (float2(1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _612 = dot(float4(_781, _809, fast::clamp(log2((length(_41.uCamPos - float3(_846.x, 0.0, _846.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _893), float4(0.25));
        float _618 = dot(float4(fast::clamp(log2((length(_41.uCamPos - float3(_678.x, 0.0, _678.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _725, _781, _809), float4(0.25));
        float _624 = dot(float4(_725, fast::clamp(log2((length(_41.uCamPos - float3(_734.x, 0.0, _734.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _809, _837), float4(0.25));
        float _630 = dot(float4(_809, _837, _893, fast::clamp(log2((length(_41.uCamPos - float3(_902.x, 0.0, _902.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x)), float4(0.25));
        float4 _631 = float4(_612, _618, _624, _630);
        patchOut.vPatchLods = _631;
        float4 _928 = exp2(-fast::min(_631, _631.yzwx)) * _41.uMaxTessLevel.y;
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(_928.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(_928.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(_928.z);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(_928.w);
        float _935 = _41.uMaxTessLevel.y * exp2(-fast::min(fast::min(fast::min(_612, _618), fast::min(_624, _630)), _809));
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(_935);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(_935);
    }
}

