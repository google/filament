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

constant spvUnsafeArray<float4, 4> _68 = spvUnsafeArray<float4, 4>({ float4(0.0), float4(1.0), float4(2.0), float4(3.0) });

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    int Index1 [[attribute(0)]];
    int Index2 [[attribute(1)]];
};

static inline __attribute__((always_inline))
float4 consume_constant_arrays2(spvUnsafeArray<float4, 4> positions, spvUnsafeArray<float4, 4> positions2, thread int& Index1, thread int& Index2)
{
    spvUnsafeArray<float4, 4> indexable = positions;
    spvUnsafeArray<float4, 4> indexable_1 = positions2;
    return indexable[Index1] + indexable_1[Index2];
}

static inline __attribute__((always_inline))
float4 consume_constant_arrays(spvUnsafeArray<float4, 4> positions, spvUnsafeArray<float4, 4> positions2, thread int& Index1, thread int& Index2)
{
    return consume_constant_arrays2(positions, positions2, Index1, Index2);
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 4> LUT2;
    LUT2[0] = float4(10.0);
    LUT2[1] = float4(11.0);
    LUT2[2] = float4(12.0);
    LUT2[3] = float4(13.0);
    out.gl_Position = consume_constant_arrays(_68, LUT2, in.Index1, in.Index2);
    return out;
}

