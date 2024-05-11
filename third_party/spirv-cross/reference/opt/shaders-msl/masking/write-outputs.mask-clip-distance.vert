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
    float4 v0 [[user(locn0)]];
    float4 v1 [[user(locn1)]];
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
};

vertex main0_out main0()
{
    main0_out out = {};
    spvUnsafeArray<float, 2> gl_ClipDistance = {};
    out.v0 = float4(1.0);
    out.v1 = float4(2.0);
    out.gl_Position = float4(3.0);
    out.gl_PointSize = 4.0;
    gl_ClipDistance[0] = 1.0;
    gl_ClipDistance[1] = 0.5;
    return out;
}

