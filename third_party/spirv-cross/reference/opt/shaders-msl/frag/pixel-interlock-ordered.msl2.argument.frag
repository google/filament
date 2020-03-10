#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

struct Buffer3
{
    int baz;
};

struct Buffer
{
    int foo;
    uint bar;
};

struct Buffer2
{
    uint quux;
};

struct spvDescriptorSetBuffer0
{
    device Buffer3* m_9 [[id(0)]];
    texture2d<float, access::write> img4 [[id(1)]];
    texture2d<float, access::write> img [[id(2), raster_order_group(0)]];
    texture2d<float> img3 [[id(3), raster_order_group(0)]];
    volatile device Buffer* m_34 [[id(4), raster_order_group(0)]];
    device Buffer2* m_44 [[id(5), raster_order_group(0)]];
};

fragment void main0(constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]])
{
    (*spvDescriptorSet0.m_9).baz = 0;
    spvDescriptorSet0.img4.write(float4(1.0, 0.0, 0.0, 1.0), uint2(int2(1)));
    spvDescriptorSet0.img.write(spvDescriptorSet0.img3.read(uint2(int2(0))), uint2(int2(0)));
    (*spvDescriptorSet0.m_34).foo += 42;
    uint _49 = atomic_fetch_and_explicit((volatile device atomic_uint*)&(*spvDescriptorSet0.m_34).bar, (*spvDescriptorSet0.m_44).quux, memory_order_relaxed);
}

