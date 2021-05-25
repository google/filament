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

struct main0_in
{
    float4 FragColors [[attribute(2)]];
    float4 gl_Position [[attribute(1)]];
};

struct main0_patchIn
{
    float4 FragColor [[attribute(0)]];
    float4 gl_TessLevelOuter [[attribute(3)]];
    float2 gl_TessLevelInner [[attribute(4)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], uint gl_PrimitiveID [[patch_id]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> gl_TessLevelInner = {};
    spvUnsafeArray<float, 4> gl_TessLevelOuter = {};
    gl_TessLevelInner[0] = patchIn.gl_TessLevelInner.x;
    gl_TessLevelInner[1] = patchIn.gl_TessLevelInner.y;
    gl_TessLevelOuter[0] = patchIn.gl_TessLevelOuter.x;
    gl_TessLevelOuter[1] = patchIn.gl_TessLevelOuter.y;
    gl_TessLevelOuter[2] = patchIn.gl_TessLevelOuter.z;
    gl_TessLevelOuter[3] = patchIn.gl_TessLevelOuter.w;
    out.gl_Position = (((((float4(1.0) + patchIn.FragColor) + patchIn.gl_in[0].FragColors) + patchIn.gl_in[1].FragColors) + float4(gl_TessLevelInner[0])) + float4(gl_TessLevelOuter[int(gl_PrimitiveID) & 1])) + patchIn.gl_in[0].gl_Position;
    return out;
}

