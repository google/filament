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

constant float _21 = {};

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    spvUnsafeArray<float, 2> _23;
    for (int _25 = 0; _25 < 2; )
    {
        _23[_25] = 0.0;
        _25++;
        continue;
    }
    float _37;
    if (as_type<uint>(3.0) != 0u)
    {
        _37 = _23[0];
    }
    else
    {
        _37 = _21;
    }
    out.gl_Position = float4(0.0, 0.0, 0.0, _37);
    return out;
}

