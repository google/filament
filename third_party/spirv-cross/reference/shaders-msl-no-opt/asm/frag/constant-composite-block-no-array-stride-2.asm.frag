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

struct _3
{
    spvUnsafeArray<float, 2> _m0;
    float _m1[2];
    spvUnsafeArray<float, 2> _m2;
};

constant spvUnsafeArray<float, 2> _15 = spvUnsafeArray<float, 2>({ 1.0, 2.0 });
constant spvUnsafeArray<float, 2> _16 = spvUnsafeArray<float, 2>({ 3.0, 4.0 });
constant spvUnsafeArray<float, 2> _17 = spvUnsafeArray<float, 2>({ 5.0, 6.0 });

struct main0_out
{
    float m_2 [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    _3 _23 = _3{ spvUnsafeArray<float, 2>({ 1.0, 2.0 }), { 3.0, 4.0 }, spvUnsafeArray<float, 2>({ 5.0, 6.0 }) };
    out.m_2 = 1.0;
    return out;
}

