#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct S
{
    int x;
    float4 y;
    spvUnsafeArray<float, 2> z;
};

struct TheBlock
{
    spvUnsafeArray<float, 3> blockFa;
    spvUnsafeArray<S, 2> blockSa;
    float blockF;
};

struct main0_patchOut
{
    float2 in_te_positionScale;
    float2 in_te_positionOffset;
    spvUnsafeArray<TheBlock, 2> tcBlock;
};

struct main0_in
{
    float3 in_tc_attr;
    ushort2 m_196;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 5];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 5, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 5, spvIndirectParams[1] - 1);
    int _163;
    _163 = 0;
    float _111;
    for (float _170 = 1.2999999523162841796875; _163 < 2; _170 = _111, _163++)
    {
        float _169;
        _169 = _170;
        for (int _164 = 0; _164 < 3; )
        {
            patchOut.tcBlock[_163].blockFa[_164] = _169;
            _169 += 0.4000000059604644775390625;
            _164++;
            continue;
        }
        int _165;
        float _168;
        _168 = _169;
        _165 = 0;
        float _174;
        for (; _165 < 2; _168 = _174, _165++)
        {
            patchOut.tcBlock[_163].blockSa[_165].x = int(_168);
            patchOut.tcBlock[_163].blockSa[_165].y = float4(_168 + 0.4000000059604644775390625, _168 + 1.2000000476837158203125, _168 + 2.0, _168 + 2.80000019073486328125);
            _174 = _168 + 0.800000011920928955078125;
            for (int _171 = 0; _171 < 2; )
            {
                patchOut.tcBlock[_163].blockSa[_165].z[_171] = _174;
                _174 += 0.4000000059604644775390625;
                _171++;
                continue;
            }
        }
        patchOut.tcBlock[_163].blockF = _168;
        _111 = _168 + 0.4000000059604644775390625;
    }
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(gl_in[0].in_tc_attr.x);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(gl_in[1].in_tc_attr.x);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(gl_in[2].in_tc_attr.x);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(gl_in[3].in_tc_attr.x);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(gl_in[4].in_tc_attr.x);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(gl_in[5].in_tc_attr.x);
    patchOut.in_te_positionScale = float2(gl_in[6].in_tc_attr.x, gl_in[7].in_tc_attr.x);
    patchOut.in_te_positionOffset = float2(gl_in[8].in_tc_attr.x, gl_in[9].in_tc_attr.x);
}

