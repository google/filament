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

struct VertexData
{
    float4x4 a;
    spvUnsafeArray<float4, 2> b;
    float4 c;
};

struct main0_out
{
    float4 vOutputs;
};

struct main0_in
{
    float4 vInputs_a_0 [[attribute(0)]];
    float4 vInputs_a_1 [[attribute(1)]];
    float4 vInputs_a_2 [[attribute(2)]];
    float4 vInputs_a_3 [[attribute(3)]];
    float4 vInputs_b_0 [[attribute(4)]];
    float4 vInputs_b_1 [[attribute(5)]];
    float4 vInputs_c [[attribute(6)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
    spvUnsafeArray<VertexData, 32> _19 = spvUnsafeArray<VertexData, 32>({ VertexData{ float4x4(gl_in[0].vInputs_a_0, gl_in[0].vInputs_a_1, gl_in[0].vInputs_a_2, gl_in[0].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[0].vInputs_b_0, gl_in[0].vInputs_b_1 }), gl_in[0].vInputs_c }, VertexData{ float4x4(gl_in[1].vInputs_a_0, gl_in[1].vInputs_a_1, gl_in[1].vInputs_a_2, gl_in[1].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[1].vInputs_b_0, gl_in[1].vInputs_b_1 }), gl_in[1].vInputs_c }, VertexData{ float4x4(gl_in[2].vInputs_a_0, gl_in[2].vInputs_a_1, gl_in[2].vInputs_a_2, gl_in[2].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[2].vInputs_b_0, gl_in[2].vInputs_b_1 }), gl_in[2].vInputs_c }, VertexData{ float4x4(gl_in[3].vInputs_a_0, gl_in[3].vInputs_a_1, gl_in[3].vInputs_a_2, gl_in[3].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[3].vInputs_b_0, gl_in[3].vInputs_b_1 }), gl_in[3].vInputs_c }, VertexData{ float4x4(gl_in[4].vInputs_a_0, gl_in[4].vInputs_a_1, gl_in[4].vInputs_a_2, gl_in[4].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[4].vInputs_b_0, gl_in[4].vInputs_b_1 }), gl_in[4].vInputs_c }, VertexData{ float4x4(gl_in[5].vInputs_a_0, gl_in[5].vInputs_a_1, gl_in[5].vInputs_a_2, gl_in[5].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[5].vInputs_b_0, gl_in[5].vInputs_b_1 }), gl_in[5].vInputs_c }, VertexData{ float4x4(gl_in[6].vInputs_a_0, gl_in[6].vInputs_a_1, gl_in[6].vInputs_a_2, gl_in[6].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[6].vInputs_b_0, gl_in[6].vInputs_b_1 }), gl_in[6].vInputs_c }, VertexData{ float4x4(gl_in[7].vInputs_a_0, gl_in[7].vInputs_a_1, gl_in[7].vInputs_a_2, gl_in[7].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[7].vInputs_b_0, gl_in[7].vInputs_b_1 }), gl_in[7].vInputs_c }, VertexData{ float4x4(gl_in[8].vInputs_a_0, gl_in[8].vInputs_a_1, gl_in[8].vInputs_a_2, gl_in[8].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[8].vInputs_b_0, gl_in[8].vInputs_b_1 }), gl_in[8].vInputs_c }, VertexData{ float4x4(gl_in[9].vInputs_a_0, gl_in[9].vInputs_a_1, gl_in[9].vInputs_a_2, gl_in[9].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[9].vInputs_b_0, gl_in[9].vInputs_b_1 }), gl_in[9].vInputs_c }, VertexData{ float4x4(gl_in[10].vInputs_a_0, gl_in[10].vInputs_a_1, gl_in[10].vInputs_a_2, gl_in[10].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[10].vInputs_b_0, gl_in[10].vInputs_b_1 }), gl_in[10].vInputs_c }, VertexData{ float4x4(gl_in[11].vInputs_a_0, gl_in[11].vInputs_a_1, gl_in[11].vInputs_a_2, gl_in[11].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[11].vInputs_b_0, gl_in[11].vInputs_b_1 }), gl_in[11].vInputs_c }, VertexData{ float4x4(gl_in[12].vInputs_a_0, gl_in[12].vInputs_a_1, gl_in[12].vInputs_a_2, gl_in[12].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[12].vInputs_b_0, gl_in[12].vInputs_b_1 }), gl_in[12].vInputs_c }, VertexData{ float4x4(gl_in[13].vInputs_a_0, gl_in[13].vInputs_a_1, gl_in[13].vInputs_a_2, gl_in[13].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[13].vInputs_b_0, gl_in[13].vInputs_b_1 }), gl_in[13].vInputs_c }, VertexData{ float4x4(gl_in[14].vInputs_a_0, gl_in[14].vInputs_a_1, gl_in[14].vInputs_a_2, gl_in[14].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[14].vInputs_b_0, gl_in[14].vInputs_b_1 }), gl_in[14].vInputs_c }, VertexData{ float4x4(gl_in[15].vInputs_a_0, gl_in[15].vInputs_a_1, gl_in[15].vInputs_a_2, gl_in[15].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[15].vInputs_b_0, gl_in[15].vInputs_b_1 }), gl_in[15].vInputs_c }, VertexData{ float4x4(gl_in[16].vInputs_a_0, gl_in[16].vInputs_a_1, gl_in[16].vInputs_a_2, gl_in[16].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[16].vInputs_b_0, gl_in[16].vInputs_b_1 }), gl_in[16].vInputs_c }, VertexData{ float4x4(gl_in[17].vInputs_a_0, gl_in[17].vInputs_a_1, gl_in[17].vInputs_a_2, gl_in[17].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[17].vInputs_b_0, gl_in[17].vInputs_b_1 }), gl_in[17].vInputs_c }, VertexData{ float4x4(gl_in[18].vInputs_a_0, gl_in[18].vInputs_a_1, gl_in[18].vInputs_a_2, gl_in[18].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[18].vInputs_b_0, gl_in[18].vInputs_b_1 }), gl_in[18].vInputs_c }, VertexData{ float4x4(gl_in[19].vInputs_a_0, gl_in[19].vInputs_a_1, gl_in[19].vInputs_a_2, gl_in[19].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[19].vInputs_b_0, gl_in[19].vInputs_b_1 }), gl_in[19].vInputs_c }, VertexData{ float4x4(gl_in[20].vInputs_a_0, gl_in[20].vInputs_a_1, gl_in[20].vInputs_a_2, gl_in[20].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[20].vInputs_b_0, gl_in[20].vInputs_b_1 }), gl_in[20].vInputs_c }, VertexData{ float4x4(gl_in[21].vInputs_a_0, gl_in[21].vInputs_a_1, gl_in[21].vInputs_a_2, gl_in[21].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[21].vInputs_b_0, gl_in[21].vInputs_b_1 }), gl_in[21].vInputs_c }, VertexData{ float4x4(gl_in[22].vInputs_a_0, gl_in[22].vInputs_a_1, gl_in[22].vInputs_a_2, gl_in[22].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[22].vInputs_b_0, gl_in[22].vInputs_b_1 }), gl_in[22].vInputs_c }, VertexData{ float4x4(gl_in[23].vInputs_a_0, gl_in[23].vInputs_a_1, gl_in[23].vInputs_a_2, gl_in[23].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[23].vInputs_b_0, gl_in[23].vInputs_b_1 }), gl_in[23].vInputs_c }, VertexData{ float4x4(gl_in[24].vInputs_a_0, gl_in[24].vInputs_a_1, gl_in[24].vInputs_a_2, gl_in[24].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[24].vInputs_b_0, gl_in[24].vInputs_b_1 }), gl_in[24].vInputs_c }, VertexData{ float4x4(gl_in[25].vInputs_a_0, gl_in[25].vInputs_a_1, gl_in[25].vInputs_a_2, gl_in[25].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[25].vInputs_b_0, gl_in[25].vInputs_b_1 }), gl_in[25].vInputs_c }, VertexData{ float4x4(gl_in[26].vInputs_a_0, gl_in[26].vInputs_a_1, gl_in[26].vInputs_a_2, gl_in[26].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[26].vInputs_b_0, gl_in[26].vInputs_b_1 }), gl_in[26].vInputs_c }, VertexData{ float4x4(gl_in[27].vInputs_a_0, gl_in[27].vInputs_a_1, gl_in[27].vInputs_a_2, gl_in[27].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[27].vInputs_b_0, gl_in[27].vInputs_b_1 }), gl_in[27].vInputs_c }, VertexData{ float4x4(gl_in[28].vInputs_a_0, gl_in[28].vInputs_a_1, gl_in[28].vInputs_a_2, gl_in[28].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[28].vInputs_b_0, gl_in[28].vInputs_b_1 }), gl_in[28].vInputs_c }, VertexData{ float4x4(gl_in[29].vInputs_a_0, gl_in[29].vInputs_a_1, gl_in[29].vInputs_a_2, gl_in[29].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[29].vInputs_b_0, gl_in[29].vInputs_b_1 }), gl_in[29].vInputs_c }, VertexData{ float4x4(gl_in[30].vInputs_a_0, gl_in[30].vInputs_a_1, gl_in[30].vInputs_a_2, gl_in[30].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[30].vInputs_b_0, gl_in[30].vInputs_b_1 }), gl_in[30].vInputs_c }, VertexData{ float4x4(gl_in[31].vInputs_a_0, gl_in[31].vInputs_a_1, gl_in[31].vInputs_a_2, gl_in[31].vInputs_a_3), spvUnsafeArray<float4, 2>({ gl_in[31].vInputs_b_0, gl_in[31].vInputs_b_1 }), gl_in[31].vInputs_c } });
    spvUnsafeArray<VertexData, 32> tmp;
    tmp = _19;
    int _27 = gl_InvocationID ^ 1;
    gl_out[gl_InvocationID].vOutputs = ((tmp[gl_InvocationID].a[1] + tmp[gl_InvocationID].b[1]) + tmp[gl_InvocationID].c) + gl_in[_27].vInputs_c;
}

