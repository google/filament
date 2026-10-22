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
    float4 FragColors_0 [[color(0)]];
    float4 FragColors_1 [[color(1)]];
    float4 FragColor2 [[color(2)]];
    float4 FragColor3 [[color(3)]];
};

struct main0_in
{
    float3 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> FragColors = {};
    float2 FragColor2 = {};
    float3 FragColor3 = {};
    FragColors[0] = in.vColor.x;
    FragColors[1] = in.vColor.y;
    FragColor2 = in.vColor.xz;
    FragColor3 = in.vColor.zzz;
    out.FragColors_0 = float4(FragColors[0]);
    out.FragColors_1 = float4(FragColors[1]);
    out.FragColor2.xy = FragColor2;
    out.FragColor3.xyz = FragColor3;
    return out;
}

