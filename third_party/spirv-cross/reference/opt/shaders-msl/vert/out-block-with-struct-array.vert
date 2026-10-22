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

struct t21
{
    float m0;
    float4 m1;
};

struct main0_out
{
    float v25_0_m0 [[user(locn0)]];
    float4 v25_0_m1 [[user(locn1)]];
    float v25_1_m0 [[user(locn2)]];
    float4 v25_1_m1 [[user(locn3)]];
    float v25_2_m0 [[user(locn4)]];
    float4 v25_2_m1 [[user(locn5)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 v17 [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<t21, 3> v25 = {};
    out.gl_Position = in.v17;
    v25[2].m1 = float4(-4.0, -9.0, 3.0, 7.0);
    out.v25_0_m0 = v25[0].m0;
    out.v25_0_m1 = v25[0].m1;
    out.v25_1_m0 = v25[1].m0;
    out.v25_1_m1 = v25[1].m1;
    out.v25_2_m0 = v25[2].m0;
    out.v25_2_m1 = v25[2].m1;
    return out;
}

