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

struct cb1_struct
{
    float4 _m0[1];
};

struct main0_out
{
    float3 vocp0;
    float4 vocp1;
};

struct main0_in
{
    float4 v0 [[attribute(0)]];
    float4 v1 [[attribute(1)]];
    float3 vicp0 [[attribute(2)]];
    float4 vicp1 [[attribute(4)]];
};

static inline __attribute__((always_inline))
void fork0_epilogue(thread const float4& _87, thread const float4& _88, thread const float4& _89, device half (&gl_TessLevelOuter)[3])
{
    gl_TessLevelOuter[0u] = half(_87.x);
    gl_TessLevelOuter[1u] = half(_88.x);
    gl_TessLevelOuter[2u] = half(_89.x);
}

static inline __attribute__((always_inline))
void fork0(uint vForkInstanceId, device half (&gl_TessLevelOuter)[3], thread spvUnsafeArray<float4, 4>& opc, constant cb1_struct& cb0_0, thread float4& v_48, thread float4& v_49, thread float4& v_50)
{
    float4 r0;
    r0.x = as_type<float>(vForkInstanceId);
    opc[as_type<int>(r0.x)].x = cb0_0._m0[0u].x;
    v_48 = opc[0u];
    v_49 = opc[1u];
    v_50 = opc[2u];
    fork0_epilogue(v_48, v_49, v_50, gl_TessLevelOuter);
}

static inline __attribute__((always_inline))
void fork1_epilogue(thread const float4& _109, device half &gl_TessLevelInner)
{
    gl_TessLevelInner = half(_109.x);
}

static inline __attribute__((always_inline))
void fork1(device half &gl_TessLevelInner, thread spvUnsafeArray<float4, 4>& opc, constant cb1_struct& cb0_0, thread float4& v_56)
{
    opc[3u].x = cb0_0._m0[0u].x;
    v_56 = opc[3u];
    fork1_epilogue(v_56, gl_TessLevelInner);
}

kernel void main0(main0_in in [[stage_in]], constant cb1_struct& cb0_0 [[buffer(0)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    spvUnsafeArray<spvUnsafeArray<float4, 3>, 2> vicp;
    spvUnsafeArray<float4, 3> _153 = spvUnsafeArray<float4, 3>({ gl_in[0].v0, gl_in[1].v0, gl_in[2].v0 });
    vicp[0u] = _153;
    spvUnsafeArray<float4, 3> _154 = spvUnsafeArray<float4, 3>({ gl_in[0].v1, gl_in[1].v1, gl_in[2].v1 });
    vicp[1u] = _154;
    gl_out[gl_InvocationID].vocp0 = gl_in[gl_InvocationID].vicp0;
    gl_out[gl_InvocationID].vocp1 = gl_in[gl_InvocationID].vicp1;
    spvUnsafeArray<float4, 4> opc;
    float4 v_48;
    float4 v_49;
    float4 v_50;
    fork0(0u, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor, opc, cb0_0, v_48, v_49, v_50);
    fork0(1u, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor, opc, cb0_0, v_48, v_49, v_50);
    fork0(2u, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor, opc, cb0_0, v_48, v_49, v_50);
    float4 v_56;
    fork1(spvTessLevel[gl_PrimitiveID].insideTessellationFactor, opc, cb0_0, v_56);
}

