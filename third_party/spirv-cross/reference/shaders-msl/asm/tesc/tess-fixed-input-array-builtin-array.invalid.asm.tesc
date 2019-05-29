#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOutput
{
    float4 pos;
    float2 uv;
};

struct HSOut
{
    float4 pos;
    float2 uv;
};

struct HSConstantOut
{
    float EdgeTess[3];
    float InsideTess;
};

struct VertexOutput_1
{
    float2 uv;
};

struct HSOut_1
{
    float2 uv;
};

struct main0_out
{
    HSOut_1 _entryPointOutput;
    float4 gl_Position;
};

struct main0_in
{
    float2 VertexOutput_uv [[attribute(0)]];
    float4 gl_Position [[attribute(1)]];
};

// Implementation of an array copy function to cover GLSL's ability to copy an array via assignment.
template<typename T, uint N>
void spvArrayCopyFromStack1(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

template<typename T, uint N>
void spvArrayCopyFromConstant1(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

HSOut _hs_main(thread const VertexOutput (&p)[3], thread const uint& i)
{
    HSOut _output;
    _output.pos = p[i].pos;
    _output.uv = p[i].uv;
    return _output;
}

HSConstantOut PatchHS(thread const VertexOutput (&_patch)[3])
{
    HSConstantOut _output;
    _output.EdgeTess[0] = (float2(1.0) + _patch[0].uv).x;
    _output.EdgeTess[1] = (float2(1.0) + _patch[0].uv).x;
    _output.EdgeTess[2] = (float2(1.0) + _patch[0].uv).x;
    _output.InsideTess = (float2(1.0) + _patch[0].uv).x;
    return _output;
}

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    VertexOutput p[3];
    p[0].pos = gl_in[0].gl_Position;
    p[0].uv = gl_in[0].VertexOutput_uv;
    p[1].pos = gl_in[1].gl_Position;
    p[1].uv = gl_in[1].VertexOutput_uv;
    p[2].pos = gl_in[2].gl_Position;
    p[2].uv = gl_in[2].VertexOutput_uv;
    uint i = gl_InvocationID;
    VertexOutput param[3];
    spvArrayCopyFromStack1(param, p);
    uint param_1 = i;
    HSOut flattenTemp = _hs_main(param, param_1);
    gl_out[gl_InvocationID].gl_Position = flattenTemp.pos;
    gl_out[gl_InvocationID]._entryPointOutput.uv = flattenTemp.uv;
    threadgroup_barrier(mem_flags::mem_device);
    if (int(gl_InvocationID) == 0)
    {
        VertexOutput param_2[3];
        spvArrayCopyFromStack1(param_2, p);
        HSConstantOut _patchConstantResult = PatchHS(param_2);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(_patchConstantResult.EdgeTess[0]);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(_patchConstantResult.EdgeTess[1]);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(_patchConstantResult.EdgeTess[2]);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_patchConstantResult.InsideTess);
    }
}

