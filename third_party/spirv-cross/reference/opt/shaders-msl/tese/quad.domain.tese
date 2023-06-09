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
    float4 gl_TessLevelOuter [[attribute(0)]];
    float2 gl_TessLevelInner [[attribute(1)]];
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float2 gl_TessCoordIn [[position_in_patch]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> gl_TessLevelInner = {};
    spvUnsafeArray<float, 4> gl_TessLevelOuter = {};
    gl_TessLevelInner[0] = patchIn.gl_TessLevelInner[0];
    gl_TessLevelInner[1] = patchIn.gl_TessLevelInner[1];
    gl_TessLevelOuter[0] = patchIn.gl_TessLevelOuter[0];
    gl_TessLevelOuter[1] = patchIn.gl_TessLevelOuter[1];
    gl_TessLevelOuter[2] = patchIn.gl_TessLevelOuter[2];
    gl_TessLevelOuter[3] = patchIn.gl_TessLevelOuter[3];
    float3 gl_TessCoord = float3(gl_TessCoordIn.x, gl_TessCoordIn.y, 0.0);
    gl_TessCoord.y = 1.0 - gl_TessCoord.y;
    out.gl_Position = float4(fma(gl_TessCoord.x * gl_TessLevelInner[0], gl_TessLevelOuter[0], ((1.0 - gl_TessCoord.x) * gl_TessLevelInner[0]) * gl_TessLevelOuter[2]), fma(gl_TessCoord.y * gl_TessLevelInner[1], gl_TessLevelOuter[3], ((1.0 - gl_TessCoord.y) * gl_TessLevelInner[1]) * gl_TessLevelOuter[1]), 0.0, 1.0);
    return out;
}

