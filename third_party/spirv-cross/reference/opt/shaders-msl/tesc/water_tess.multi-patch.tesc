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
    ushort2 m_996;
};

kernel void main0(constant UBO& _41 [[buffer(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 1];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1]);
    float2 _431 = (gl_in[0].vPatchPosBase.xy - float2(10.0)) * _41.uScale.xy;
    float2 _441 = ((gl_in[0].vPatchPosBase.xy + _41.uPatchSize) + float2(10.0)) * _41.uScale.xy;
    float3 _446 = float3(_431.x, -10.0, _431.y);
    float3 _451 = float3(_441.x, 10.0, _441.y);
    float4 _467 = float4((_446 + _451) * 0.5, 1.0);
    float3 _514 = float3(length(_451 - _446) * (-0.5));
    bool _516 = any(float3(dot(_41.uFrustum[0], _467), dot(_41.uFrustum[1], _467), dot(_41.uFrustum[2], _467)) <= _514);
    bool _526;
    if (!_516)
    {
        _526 = any(float3(dot(_41.uFrustum[3], _467), dot(_41.uFrustum[4], _467), dot(_41.uFrustum[5], _467)) <= _514);
    }
    else
    {
        _526 = _516;
    }
    if (!(!_526))
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
        patchOut.vOutPatchPosBase = gl_in[0].vPatchPosBase.xy;
        float2 _681 = (gl_in[0].vPatchPosBase.xy + (float2(-0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float2 _710 = (gl_in[0].vPatchPosBase.xy + (float2(0.5, -0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _729 = fast::clamp(log2((length(_41.uCamPos - float3(_710.x, 0.0, _710.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _739 = (gl_in[0].vPatchPosBase.xy + (float2(1.5, -0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float2 _768 = (gl_in[0].vPatchPosBase.xy + (float2(-0.5, 0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _787 = fast::clamp(log2((length(_41.uCamPos - float3(_768.x, 0.0, _768.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _797 = (gl_in[0].vPatchPosBase.xy + (float2(0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _816 = fast::clamp(log2((length(_41.uCamPos - float3(_797.x, 0.0, _797.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _826 = (gl_in[0].vPatchPosBase.xy + (float2(1.5, 0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _845 = fast::clamp(log2((length(_41.uCamPos - float3(_826.x, 0.0, _826.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _855 = (gl_in[0].vPatchPosBase.xy + (float2(-0.5, 1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float2 _884 = (gl_in[0].vPatchPosBase.xy + (float2(0.5, 1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _903 = fast::clamp(log2((length(_41.uCamPos - float3(_884.x, 0.0, _884.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        float2 _913 = (gl_in[0].vPatchPosBase.xy + (float2(1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _614 = dot(float4(_787, _816, fast::clamp(log2((length(_41.uCamPos - float3(_855.x, 0.0, _855.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _903), float4(0.25));
        float _620 = dot(float4(fast::clamp(log2((length(_41.uCamPos - float3(_681.x, 0.0, _681.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _729, _787, _816), float4(0.25));
        float _626 = dot(float4(_729, fast::clamp(log2((length(_41.uCamPos - float3(_739.x, 0.0, _739.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _816, _845), float4(0.25));
        float _632 = dot(float4(_816, _845, _903, fast::clamp(log2((length(_41.uCamPos - float3(_913.x, 0.0, _913.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x)), float4(0.25));
        float4 _633 = float4(_614, _620, _626, _632);
        patchOut.vPatchLods = _633;
        float4 _940 = exp2(-fast::min(_633, _633.yzwx)) * _41.uMaxTessLevel.y;
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(_940.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(_940.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(_940.z);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(_940.w);
        float _948 = _41.uMaxTessLevel.y * exp2(-fast::min(fast::min(fast::min(_614, _620), fast::min(_626, _632)), _816));
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(_948);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(_948);
    }
}

