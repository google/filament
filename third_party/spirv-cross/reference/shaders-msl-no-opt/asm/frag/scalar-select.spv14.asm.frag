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

struct _16
{
    float _m0;
};

constant spvUnsafeArray<float, 2> _34 = spvUnsafeArray<float, 2>({ 0.0, 1.0 });
constant spvUnsafeArray<float, 2> _35 = spvUnsafeArray<float, 2>({ 1.0, 0.0 });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = false ? float4(1.0, 1.0, 0.0, 1.0) : float4(0.0, 0.0, 0.0, 1.0);
    out.FragColor = float4(false);
    out.FragColor = select(float4(0.0, 0.0, 0.0, 1.0), float4(1.0, 1.0, 0.0, 1.0), bool4(false, true, false, true));
    out.FragColor = float4(bool4(false, true, false, true));
    _16 _36 = false ? (_16{ 0.0 }) : (_16{ 1.0 });
    spvUnsafeArray<float, 2> _37 = true ? _34 : _35;
    return out;
}

