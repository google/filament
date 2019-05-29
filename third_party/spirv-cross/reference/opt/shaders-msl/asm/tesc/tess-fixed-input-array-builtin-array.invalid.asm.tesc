#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOutput
{
    float4 pos;
    float2 uv;
};

struct VertexOutput_1
{
    float2 uv;
};

struct HSOut
{
    float2 uv;
};

struct main0_out
{
    HSOut _entryPointOutput;
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

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    VertexOutput _223[3] = { VertexOutput{ gl_in[0].gl_Position, gl_in[0].VertexOutput_uv }, VertexOutput{ gl_in[1].gl_Position, gl_in[1].VertexOutput_uv }, VertexOutput{ gl_in[2].gl_Position, gl_in[2].VertexOutput_uv } };
    VertexOutput param[3];
    spvArrayCopyFromStack1(param, _223);
    gl_out[gl_InvocationID].gl_Position = param[gl_InvocationID].pos;
    gl_out[gl_InvocationID]._entryPointOutput.uv = param[gl_InvocationID].uv;
    threadgroup_barrier(mem_flags::mem_device);
    if (int(gl_InvocationID) == 0)
    {
        float2 _174 = float2(1.0) + gl_in[0].VertexOutput_uv;
        float _175 = _174.x;
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(_175);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(_175);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(_175);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_175);
    }
}

