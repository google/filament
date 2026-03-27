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

struct Vertex
{
    float2 umbrage;
    spvUnsafeArray<float4, 3> offset;
};

struct main0_out
{
    float2 aa_umbrage [[user(locn1)]];
    float4 aa_offset_0 [[user(locn2)]];
    float4 aa_offset_1 [[user(locn3)]];
    float4 aa_offset_2 [[user(locn4)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 dd_0 [[attribute(3)]];
};

static inline __attribute__((always_inline))
void Wobble(thread const float2& uu, thread spvUnsafeArray<float4, 3>& offset)
{
    offset[0] = float4(uu.xyxy);
    offset[1] = float4(uu.xyxy);
    offset[2] = float4(uu.xyxy);
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Vertex aa = {};
    spvUnsafeArray<float3, 1> dd = {};
    dd[0] = in.dd_0;
    float2 param = dd[0].xy;
    spvUnsafeArray<float4, 3> param_1;
    Wobble(param, param_1);
    aa.offset = param_1;
    out.gl_Position = float4(1.0, 1.0, 0.0, 1.0);
    out.aa_umbrage = aa.umbrage;
    out.aa_offset_0 = aa.offset[0];
    out.aa_offset_1 = aa.offset[1];
    out.aa_offset_2 = aa.offset[2];
    return out;
}

