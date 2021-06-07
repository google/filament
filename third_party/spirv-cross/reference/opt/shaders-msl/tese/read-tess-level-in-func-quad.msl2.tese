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

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float, 4> gl_TessLevelOuter = {};
    spvUnsafeArray<float, 2> gl_TessLevelInner = {};
    gl_TessLevelOuter[0] = patchIn.gl_TessLevelOuter.x;
    gl_TessLevelOuter[1] = patchIn.gl_TessLevelOuter.y;
    gl_TessLevelOuter[2] = patchIn.gl_TessLevelOuter.z;
    gl_TessLevelOuter[3] = patchIn.gl_TessLevelOuter.w;
    gl_TessLevelInner[0] = patchIn.gl_TessLevelInner.x;
    gl_TessLevelInner[1] = patchIn.gl_TessLevelInner.y;
    out.gl_Position = float4(gl_TessLevelOuter[0], gl_TessLevelOuter[1], gl_TessLevelOuter[2], gl_TessLevelOuter[3]) + float2(gl_TessLevelInner[0], gl_TessLevelInner[1]).xyxy;
    return out;
}

