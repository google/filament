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
    ushort2 a [[attribute(0)]];
    uint3 b [[attribute(1)]];
    ushort c_0 [[attribute(2)]];
    ushort c_1 [[attribute(3)]];
    uint4 d_0 [[attribute(4)]];
    uint4 d_1 [[attribute(5)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<ushort, 2> c = {};
    spvUnsafeArray<uint4, 2> d = {};
    c[0] = in.c_0;
    c[1] = in.c_1;
    d[0] = in.d_0;
    d[1] = in.d_1;
    out.gl_Position = float4(float(int(in.a.x)), float(in.b.x), float(uint(c[1])), float(d[0].w));
    return out;
}

