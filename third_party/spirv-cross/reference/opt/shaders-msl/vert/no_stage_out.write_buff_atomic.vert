#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

struct _23
{
    uint _m0;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_17 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], device _23& _25 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = in.m_17;
    uint _29 = atomic_fetch_add_explicit((volatile device atomic_uint*)&_25._m0, 1u, memory_order_relaxed);
}

