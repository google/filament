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
    float4 VertexData_a_0 [[attribute(0)]];
    float4 VertexData_a_1 [[attribute(1)]];
    float4 VertexData_a_2 [[attribute(2)]];
    float4 VertexData_a_3 [[attribute(3)]];
    float4 VertexData_b_0 [[attribute(4)]];
    float4 VertexData_b_1 [[attribute(5)]];
    float4 VertexData_c [[attribute(6)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
    spvUnsafeArray<VertexData, 32> _19 = spvUnsafeArray<VertexData, 32>({ VertexData{ float4x4(gl_in[0].VertexData_a_0, gl_in[0].VertexData_a_1, gl_in[0].VertexData_a_2, gl_in[0].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[0].VertexData_b_0, gl_in[0].VertexData_b_1 }), gl_in[0].VertexData_c }, VertexData{ float4x4(gl_in[1].VertexData_a_0, gl_in[1].VertexData_a_1, gl_in[1].VertexData_a_2, gl_in[1].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[1].VertexData_b_0, gl_in[1].VertexData_b_1 }), gl_in[1].VertexData_c }, VertexData{ float4x4(gl_in[2].VertexData_a_0, gl_in[2].VertexData_a_1, gl_in[2].VertexData_a_2, gl_in[2].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[2].VertexData_b_0, gl_in[2].VertexData_b_1 }), gl_in[2].VertexData_c }, VertexData{ float4x4(gl_in[3].VertexData_a_0, gl_in[3].VertexData_a_1, gl_in[3].VertexData_a_2, gl_in[3].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[3].VertexData_b_0, gl_in[3].VertexData_b_1 }), gl_in[3].VertexData_c }, VertexData{ float4x4(gl_in[4].VertexData_a_0, gl_in[4].VertexData_a_1, gl_in[4].VertexData_a_2, gl_in[4].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[4].VertexData_b_0, gl_in[4].VertexData_b_1 }), gl_in[4].VertexData_c }, VertexData{ float4x4(gl_in[5].VertexData_a_0, gl_in[5].VertexData_a_1, gl_in[5].VertexData_a_2, gl_in[5].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[5].VertexData_b_0, gl_in[5].VertexData_b_1 }), gl_in[5].VertexData_c }, VertexData{ float4x4(gl_in[6].VertexData_a_0, gl_in[6].VertexData_a_1, gl_in[6].VertexData_a_2, gl_in[6].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[6].VertexData_b_0, gl_in[6].VertexData_b_1 }), gl_in[6].VertexData_c }, VertexData{ float4x4(gl_in[7].VertexData_a_0, gl_in[7].VertexData_a_1, gl_in[7].VertexData_a_2, gl_in[7].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[7].VertexData_b_0, gl_in[7].VertexData_b_1 }), gl_in[7].VertexData_c }, VertexData{ float4x4(gl_in[8].VertexData_a_0, gl_in[8].VertexData_a_1, gl_in[8].VertexData_a_2, gl_in[8].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[8].VertexData_b_0, gl_in[8].VertexData_b_1 }), gl_in[8].VertexData_c }, VertexData{ float4x4(gl_in[9].VertexData_a_0, gl_in[9].VertexData_a_1, gl_in[9].VertexData_a_2, gl_in[9].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[9].VertexData_b_0, gl_in[9].VertexData_b_1 }), gl_in[9].VertexData_c }, VertexData{ float4x4(gl_in[10].VertexData_a_0, gl_in[10].VertexData_a_1, gl_in[10].VertexData_a_2, gl_in[10].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[10].VertexData_b_0, gl_in[10].VertexData_b_1 }), gl_in[10].VertexData_c }, VertexData{ float4x4(gl_in[11].VertexData_a_0, gl_in[11].VertexData_a_1, gl_in[11].VertexData_a_2, gl_in[11].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[11].VertexData_b_0, gl_in[11].VertexData_b_1 }), gl_in[11].VertexData_c }, VertexData{ float4x4(gl_in[12].VertexData_a_0, gl_in[12].VertexData_a_1, gl_in[12].VertexData_a_2, gl_in[12].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[12].VertexData_b_0, gl_in[12].VertexData_b_1 }), gl_in[12].VertexData_c }, VertexData{ float4x4(gl_in[13].VertexData_a_0, gl_in[13].VertexData_a_1, gl_in[13].VertexData_a_2, gl_in[13].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[13].VertexData_b_0, gl_in[13].VertexData_b_1 }), gl_in[13].VertexData_c }, VertexData{ float4x4(gl_in[14].VertexData_a_0, gl_in[14].VertexData_a_1, gl_in[14].VertexData_a_2, gl_in[14].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[14].VertexData_b_0, gl_in[14].VertexData_b_1 }), gl_in[14].VertexData_c }, VertexData{ float4x4(gl_in[15].VertexData_a_0, gl_in[15].VertexData_a_1, gl_in[15].VertexData_a_2, gl_in[15].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[15].VertexData_b_0, gl_in[15].VertexData_b_1 }), gl_in[15].VertexData_c }, VertexData{ float4x4(gl_in[16].VertexData_a_0, gl_in[16].VertexData_a_1, gl_in[16].VertexData_a_2, gl_in[16].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[16].VertexData_b_0, gl_in[16].VertexData_b_1 }), gl_in[16].VertexData_c }, VertexData{ float4x4(gl_in[17].VertexData_a_0, gl_in[17].VertexData_a_1, gl_in[17].VertexData_a_2, gl_in[17].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[17].VertexData_b_0, gl_in[17].VertexData_b_1 }), gl_in[17].VertexData_c }, VertexData{ float4x4(gl_in[18].VertexData_a_0, gl_in[18].VertexData_a_1, gl_in[18].VertexData_a_2, gl_in[18].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[18].VertexData_b_0, gl_in[18].VertexData_b_1 }), gl_in[18].VertexData_c }, VertexData{ float4x4(gl_in[19].VertexData_a_0, gl_in[19].VertexData_a_1, gl_in[19].VertexData_a_2, gl_in[19].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[19].VertexData_b_0, gl_in[19].VertexData_b_1 }), gl_in[19].VertexData_c }, VertexData{ float4x4(gl_in[20].VertexData_a_0, gl_in[20].VertexData_a_1, gl_in[20].VertexData_a_2, gl_in[20].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[20].VertexData_b_0, gl_in[20].VertexData_b_1 }), gl_in[20].VertexData_c }, VertexData{ float4x4(gl_in[21].VertexData_a_0, gl_in[21].VertexData_a_1, gl_in[21].VertexData_a_2, gl_in[21].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[21].VertexData_b_0, gl_in[21].VertexData_b_1 }), gl_in[21].VertexData_c }, VertexData{ float4x4(gl_in[22].VertexData_a_0, gl_in[22].VertexData_a_1, gl_in[22].VertexData_a_2, gl_in[22].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[22].VertexData_b_0, gl_in[22].VertexData_b_1 }), gl_in[22].VertexData_c }, VertexData{ float4x4(gl_in[23].VertexData_a_0, gl_in[23].VertexData_a_1, gl_in[23].VertexData_a_2, gl_in[23].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[23].VertexData_b_0, gl_in[23].VertexData_b_1 }), gl_in[23].VertexData_c }, VertexData{ float4x4(gl_in[24].VertexData_a_0, gl_in[24].VertexData_a_1, gl_in[24].VertexData_a_2, gl_in[24].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[24].VertexData_b_0, gl_in[24].VertexData_b_1 }), gl_in[24].VertexData_c }, VertexData{ float4x4(gl_in[25].VertexData_a_0, gl_in[25].VertexData_a_1, gl_in[25].VertexData_a_2, gl_in[25].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[25].VertexData_b_0, gl_in[25].VertexData_b_1 }), gl_in[25].VertexData_c }, VertexData{ float4x4(gl_in[26].VertexData_a_0, gl_in[26].VertexData_a_1, gl_in[26].VertexData_a_2, gl_in[26].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[26].VertexData_b_0, gl_in[26].VertexData_b_1 }), gl_in[26].VertexData_c }, VertexData{ float4x4(gl_in[27].VertexData_a_0, gl_in[27].VertexData_a_1, gl_in[27].VertexData_a_2, gl_in[27].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[27].VertexData_b_0, gl_in[27].VertexData_b_1 }), gl_in[27].VertexData_c }, VertexData{ float4x4(gl_in[28].VertexData_a_0, gl_in[28].VertexData_a_1, gl_in[28].VertexData_a_2, gl_in[28].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[28].VertexData_b_0, gl_in[28].VertexData_b_1 }), gl_in[28].VertexData_c }, VertexData{ float4x4(gl_in[29].VertexData_a_0, gl_in[29].VertexData_a_1, gl_in[29].VertexData_a_2, gl_in[29].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[29].VertexData_b_0, gl_in[29].VertexData_b_1 }), gl_in[29].VertexData_c }, VertexData{ float4x4(gl_in[30].VertexData_a_0, gl_in[30].VertexData_a_1, gl_in[30].VertexData_a_2, gl_in[30].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[30].VertexData_b_0, gl_in[30].VertexData_b_1 }), gl_in[30].VertexData_c }, VertexData{ float4x4(gl_in[31].VertexData_a_0, gl_in[31].VertexData_a_1, gl_in[31].VertexData_a_2, gl_in[31].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[31].VertexData_b_0, gl_in[31].VertexData_b_1 }), gl_in[31].VertexData_c } });
    spvUnsafeArray<VertexData, 32> tmp;
    tmp = _19;
    int _27 = gl_InvocationID ^ 1;
    VertexData _30 = VertexData{ float4x4(gl_in[_27].VertexData_a_0, gl_in[_27].VertexData_a_1, gl_in[_27].VertexData_a_2, gl_in[_27].VertexData_a_3), spvUnsafeArray<float4, 2>({ gl_in[_27].VertexData_b_0, gl_in[_27].VertexData_b_1 }), gl_in[_27].VertexData_c };
    VertexData tmp_single = _30;
    gl_out[gl_InvocationID].vOutputs = ((tmp[gl_InvocationID].a[1] + tmp[gl_InvocationID].b[1]) + tmp[gl_InvocationID].c) + tmp_single.c;
}

