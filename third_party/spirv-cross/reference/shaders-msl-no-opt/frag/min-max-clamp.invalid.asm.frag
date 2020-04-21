#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    float v1 [[user(locn0)]];
    float2 v2 [[user(locn1)]];
    float3 v3 [[user(locn2)]];
    float4 v4 [[user(locn3)]];
    half h1 [[user(locn4)]];
    half2 h2 [[user(locn5)]];
    half3 h3 [[user(locn6)]];
    half4 h4 [[user(locn7)]];
};

fragment void main0(main0_in in [[stage_in]])
{
    float res = fast::min(in.v1, in.v1);
    res = fast::max(in.v1, in.v1);
    res = fast::clamp(in.v1, in.v1, in.v1);
    res = precise::min(in.v1, in.v1);
    res = precise::max(in.v1, in.v1);
    res = precise::clamp(in.v1, in.v1, in.v1);
    float2 res2 = fast::min(in.v2, in.v2);
    res2 = fast::max(in.v2, in.v2);
    res2 = fast::clamp(in.v2, in.v2, in.v2);
    res2 = precise::min(in.v2, in.v2);
    res2 = precise::max(in.v2, in.v2);
    res2 = precise::clamp(in.v2, in.v2, in.v2);
    float3 res3 = fast::min(in.v3, in.v3);
    res3 = fast::max(in.v3, in.v3);
    res3 = fast::clamp(in.v3, in.v3, in.v3);
    res3 = precise::min(in.v3, in.v3);
    res3 = precise::max(in.v3, in.v3);
    res3 = precise::clamp(in.v3, in.v3, in.v3);
    float4 res4 = fast::min(in.v4, in.v4);
    res4 = fast::max(in.v4, in.v4);
    res4 = fast::clamp(in.v4, in.v4, in.v4);
    res4 = precise::min(in.v4, in.v4);
    res4 = precise::max(in.v4, in.v4);
    res4 = precise::clamp(in.v4, in.v4, in.v4);
    half hres = min(in.h1, in.h1);
    hres = max(in.h1, in.h1);
    hres = clamp(in.h1, in.h1, in.h1);
    hres = min(in.h1, in.h1);
    hres = max(in.h1, in.h1);
    hres = clamp(in.h1, in.h1, in.h1);
    half2 hres2 = min(in.h2, in.h2);
    hres2 = max(in.h2, in.h2);
    hres2 = clamp(in.h2, in.h2, in.h2);
    hres2 = min(in.h2, in.h2);
    hres2 = max(in.h2, in.h2);
    hres2 = clamp(in.h2, in.h2, in.h2);
    half3 hres3 = min(in.h3, in.h3);
    hres3 = max(in.h3, in.h3);
    hres3 = clamp(in.h3, in.h3, in.h3);
    hres3 = min(in.h3, in.h3);
    hres3 = max(in.h3, in.h3);
    hres3 = clamp(in.h3, in.h3, in.h3);
    half4 hres4 = min(in.h4, in.h4);
    hres4 = max(in.h4, in.h4);
    hres4 = clamp(in.h4, in.h4, in.h4);
    hres4 = min(in.h4, in.h4);
    hres4 = max(in.h4, in.h4);
    hres4 = clamp(in.h4, in.h4, in.h4);
}

