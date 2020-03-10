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

struct Foo
{
    float a;
    float b;
};

constant spvUnsafeArray<float, 4> _16 = spvUnsafeArray<float, 4>({ 1.0, 4.0, 3.0, 2.0 });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int line [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    spvUnsafeArray<Foo, 2> _28 = spvUnsafeArray<Foo, 2>({ Foo{ 10.0, 20.0 }, Foo{ 30.0, 40.0 } });
    
    main0_out out = {};
    out.FragColor = float4(_16[in.line]);
    out.FragColor += float4(_28[in.line].a * _28[1 - in.line].a);
    return out;
}

