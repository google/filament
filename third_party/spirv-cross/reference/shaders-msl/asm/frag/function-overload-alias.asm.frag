#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float4 foo(thread const float4& foo_1)
{
    return foo_1 + float4(1.0);
}

static inline __attribute__((always_inline))
float4 foo(thread const float3& foo_1)
{
    return foo_1.xyzz + float4(1.0);
}

static inline __attribute__((always_inline))
float4 foo_1(thread const float4& foo_2)
{
    return foo_2 + float4(2.0);
}

static inline __attribute__((always_inline))
float4 foo(thread const float2& foo_2)
{
    return foo_2.xyxy + float4(2.0);
}

fragment main0_out main0()
{
    main0_out out = {};
    float4 foo_3 = float4(1.0);
    float4 foo_2 = foo(foo_3);
    float3 foo_5 = float3(1.0);
    float4 foo_4 = foo(foo_5);
    float4 foo_7 = float4(1.0);
    float4 foo_6 = foo_1(foo_7);
    float2 foo_9 = float2(1.0);
    float4 foo_8 = foo(foo_9);
    out.FragColor = ((foo_2 + foo_4) + foo_6) + foo_8;
    return out;
}

