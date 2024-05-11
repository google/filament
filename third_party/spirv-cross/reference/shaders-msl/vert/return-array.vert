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

constant spvUnsafeArray<float4, 2> _20 = spvUnsafeArray<float4, 2>({ float4(10.0), float4(20.0) });

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vInput0 [[attribute(0)]];
    float4 vInput1 [[attribute(1)]];
};

static inline __attribute__((always_inline))
spvUnsafeArray<float4, 2> test()
{
    return _20;
}

static inline __attribute__((always_inline))
spvUnsafeArray<float4, 2> test2(thread float4& vInput0, thread float4& vInput1)
{
    spvUnsafeArray<float4, 2> foobar;
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    return foobar;
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = test()[0] + test2(in.vInput0, in.vInput1)[1];
    return out;
}

