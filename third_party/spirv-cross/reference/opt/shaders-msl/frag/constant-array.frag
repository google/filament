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

struct Foobar
{
    float a;
    float b;
};

constant spvUnsafeArray<float4, 3> _40 = spvUnsafeArray<float4, 3>({ float4(1.0), float4(2.0), float4(3.0) });
constant spvUnsafeArray<float4, 2> _51 = spvUnsafeArray<float4, 2>({ float4(1.0), float4(2.0) });
constant spvUnsafeArray<float4, 2> _56 = spvUnsafeArray<float4, 2>({ float4(8.0), float4(10.0) });
constant spvUnsafeArray<spvUnsafeArray<float4, 2>, 2> _57 = spvUnsafeArray<spvUnsafeArray<float4, 2>, 2>({ spvUnsafeArray<float4, 2>({ float4(1.0), float4(2.0) }), spvUnsafeArray<float4, 2>({ float4(8.0), float4(10.0) }) });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    spvUnsafeArray<Foobar, 2> _77 = spvUnsafeArray<Foobar, 2>({ Foobar{ 10.0, 40.0 }, Foobar{ 90.0, 70.0 } });
    
    main0_out out = {};
    out.FragColor = ((_40[in.index] + _57[in.index][in.index + 1]) + float4(30.0)) + float4(_77[in.index].a + _77[in.index].b);
    return out;
}

