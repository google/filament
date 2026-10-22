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

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> gl_TessLevelInner = {};
    spvUnsafeArray<float, 4> gl_TessLevelOuter = {};
    gl_TessLevelInner[0] = patchIn.gl_TessLevel[3];
    gl_TessLevelOuter[0] = patchIn.gl_TessLevel[0];
    gl_TessLevelOuter[1] = patchIn.gl_TessLevel[1];
    gl_TessLevelOuter[2] = patchIn.gl_TessLevel[2];
    out.gl_Position = float4((gl_TessCoord.x * gl_TessLevelInner[0]) * gl_TessLevelOuter[0], (gl_TessCoord.y * gl_TessLevelInner[0]) * gl_TessLevelOuter[1], (gl_TessCoord.z * gl_TessLevelInner[0]) * gl_TessLevelOuter[2], 1.0);
    return out;
}

