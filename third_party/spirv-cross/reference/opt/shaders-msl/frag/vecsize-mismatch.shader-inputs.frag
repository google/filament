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
    ushort2 a [[user(locn0)]];
    uint3 b [[user(locn1)]];
    ushort c_0 [[user(locn2)]];
    ushort c_1 [[user(locn3)]];
    uint4 e_0 [[user(locn4)]];
    uint4 e_1 [[user(locn5)]];
    float4 d [[user(locn6)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<ushort, 2> c = {};
    spvUnsafeArray<uint4, 2> e = {};
    c[0] = in.c_0;
    c[1] = in.c_1;
    e[0] = in.e_0;
    e[1] = in.e_1;
    out.FragColor = float4(float(int(in.a.x)), float(in.b.x), float2(float(uint(c[1])), float(e[0].w)) + in.d.xy);
    return out;
}

