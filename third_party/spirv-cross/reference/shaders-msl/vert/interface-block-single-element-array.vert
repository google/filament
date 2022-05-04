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

struct TDPickVertex
{
    float4 c;
    spvUnsafeArray<float3, 1> uv;
};

struct main0_out
{
    float4 oTDVert_c [[user(locn0)]];
    float3 oTDVert_uv_0 [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 P [[attribute(0)]];
    float3 uv_0 [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    TDPickVertex oTDVert = {};
    spvUnsafeArray<float3, 1> uv = {};
    uv[0] = in.uv_0;
    out.gl_Position = float4(in.P, 1.0);
    oTDVert.uv[0] = uv[0];
    oTDVert.c = float4(1.0);
    out.oTDVert_c = oTDVert.c;
    out.oTDVert_uv_0 = oTDVert.uv[0];
    return out;
}

