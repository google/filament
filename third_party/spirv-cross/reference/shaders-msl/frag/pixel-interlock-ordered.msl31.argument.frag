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
    texture2d<uint, access::read_write> img2 [[id(4), raster_order_group(0)]];
    volatile device Buffer* m_42 [[id(5), raster_order_group(0)]];
    device Buffer2* m_52 [[id(6), raster_order_group(0)]];
};

fragment void main0(constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]])
{
    (*spvDescriptorSet0.m_9).baz = 0;
    spvDescriptorSet0.img4.write(float4(1.0, 0.0, 0.0, 1.0), uint2(int2(1)));
    spvDescriptorSet0.img.write(spvDescriptorSet0.img3.read(uint2(int2(0))), uint2(int2(0)));
    uint _39 = spvDescriptorSet0.img2.atomic_fetch_add(uint2(int2(0)), 1u).x;
    (*spvDescriptorSet0.m_42).foo += 42;
    uint _55 = atomic_fetch_and_explicit((volatile device atomic_uint*)&(*spvDescriptorSet0.m_42).bar, (*spvDescriptorSet0.m_52).quux, memory_order_relaxed);
}

