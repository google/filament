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

struct _14
{
    float _m0[3];
};

struct _15
{
    float _m0[3];
};

constant spvUnsafeArray<float, 3> _93 = spvUnsafeArray<float, 3>({ 1.0, 2.0, 1.0 });
constant spvUnsafeArray<float, 3> _94 = spvUnsafeArray<float, 3>({ -1.0, -2.0, -1.0 });

struct main0_out
{
    float4 m_4 [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_3 [[attribute(0)]];
    float4 m_5 [[attribute(1)]];
};

static inline __attribute__((always_inline))
float4 _102(float4 _107)
{
    float4 _109 = _107;
    _14 _110 = _14{ { 1.0, 2.0, 1.0 } };
    _15 _111 = _15{ { -1.0, -2.0, -1.0 } };
    _109.y = (_110._m0[2] + _111._m0[2]) + _109.y;
    return _109;
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.m_3;
    out.m_4 = _102(in.m_5);
    return out;
}

