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

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float4 gl_TessLevel [[attribute(0)]];
};

static inline __attribute__((always_inline))
float4 read_tess_levels(thread spvUnsafeArray<float, 4>& gl_TessLevelOuter, thread spvUnsafeArray<float, 2>& gl_TessLevelInner)
{
    return float4(gl_TessLevelOuter[0], gl_TessLevelOuter[1], gl_TessLevelOuter[2], gl_TessLevelOuter[3]) + float2(gl_TessLevelInner[0], gl_TessLevelInner[1]).xyxy;
}

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float, 4> gl_TessLevelOuter = {};
    spvUnsafeArray<float, 2> gl_TessLevelInner = {};
    gl_TessLevelOuter[0] = patchIn.gl_TessLevel[0];
    gl_TessLevelOuter[1] = patchIn.gl_TessLevel[1];
    gl_TessLevelOuter[2] = patchIn.gl_TessLevel[2];
    gl_TessLevelInner[0] = patchIn.gl_TessLevel[3];
    out.gl_Position = read_tess_levels(gl_TessLevelOuter, gl_TessLevelInner);
    return out;
}

