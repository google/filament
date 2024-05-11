#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
};

static inline __attribute__((always_inline))
void store_tess_level_in_func(device half (&gl_TessLevelInner)[2], device half (&gl_TessLevelOuter)[4])
{
    gl_TessLevelInner[0] = half(1.0);
    gl_TessLevelInner[1] = half(2.0);
    gl_TessLevelOuter[0] = half(3.0);
    gl_TessLevelOuter[1] = half(4.0);
    gl_TessLevelOuter[2] = half(5.0);
    gl_TessLevelOuter[3] = half(6.0);
}

static inline __attribute__((always_inline))
float load_tess_level_in_func(device half (&gl_TessLevelInner)[2], device half (&gl_TessLevelOuter)[4])
{
    return float(gl_TessLevelInner[0]) + float(gl_TessLevelOuter[1]);
}

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 1];
    store_tess_level_in_func(spvTessLevel[gl_PrimitiveID].insideTessellationFactor, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor);
    float v = load_tess_level_in_func(spvTessLevel[gl_PrimitiveID].insideTessellationFactor, spvTessLevel[gl_PrimitiveID].edgeTessellationFactor);
    gl_out[gl_InvocationID].gl_Position = float4(v);
}

