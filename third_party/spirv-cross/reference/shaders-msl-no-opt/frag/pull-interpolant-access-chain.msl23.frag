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
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    interpolant<float4, interpolation::perspective> a_0 [[user(locn0)]];
    interpolant<float4, interpolation::perspective> a_1 [[user(locn1)]];
    interpolant<float4, interpolation::perspective> b_0 [[user(locn2)]];
    interpolant<float4, interpolation::perspective> b_1 [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 2> a = {};
    spvUnsafeArray<float4, 2> b = {};
    a[0] = in.a_0.interpolate_at_centroid();
    a[1] = in.a_1.interpolate_at_centroid();
    b[0] = in.b_0.interpolate_at_centroid();
    b[1] = in.b_1.interpolate_at_centroid();
    out.FragColor.x = in.a_0.interpolate_at_offset(float2(0.5) + 0.4375).x;
    out.FragColor.y = in.a_1.interpolate_at_offset(float2(0.5) + 0.4375).y;
    out.FragColor.z = in.b_0.interpolate_at_offset(float2(0.5) + 0.4375).z;
    out.FragColor.w = in.b_1.interpolate_at_offset(float2(0.5) + 0.4375).w;
    return out;
}

