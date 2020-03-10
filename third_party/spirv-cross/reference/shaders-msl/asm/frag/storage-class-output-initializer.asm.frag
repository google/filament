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

constant spvUnsafeArray<float4, 2> _20 = spvUnsafeArray<float4, 2>({ float4(1.0, 2.0, 3.0, 4.0), float4(10.0) });

struct main0_out
{
    float4 FragColors_0 [[color(0)]];
    float4 FragColors_1 [[color(1)]];
    float4 FragColor [[color(2)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    spvUnsafeArray<float4, 2> FragColors = spvUnsafeArray<float4, 2>({ float4(1.0, 2.0, 3.0, 4.0), float4(10.0) });
    out.FragColor = float4(5.0);
    out.FragColors_0 = FragColors[0];
    out.FragColors_1 = FragColors[1];
    return out;
}

