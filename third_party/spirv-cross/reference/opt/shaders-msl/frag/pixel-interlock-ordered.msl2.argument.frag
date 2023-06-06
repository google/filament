#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

// The required alignment of a linear texture of R32Uint format.
constant uint spvLinearTextureAlignmentOverride [[function_constant(65535)]];
constant uint spvLinearTextureAlignment = is_function_constant_defined(spvLinearTextureAlignmentOverride) ? spvLinearTextureAlignmentOverride : 4;
// Returns buffer coords corresponding to 2D texture coords for emulating 2D texture atomics
#define spvImage2DAtomicCoord(tc, tex) (((((tex).get_width() +  spvLinearTextureAlignment / 4 - 1) & ~( spvLinearTextureAlignment / 4 - 1)) * (tc).y) + (tc).x)

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
    texture2d<uint> img2 [[id(4), raster_order_group(0)]];
    device atomic_uint* img2_atomic [[id(5), raster_order_group(0)]];
    volatile device Buffer* m_42 [[id(6), raster_order_group(0)]];
    device Buffer2* m_52 [[id(7), raster_order_group(0)]];
};

fragment void main0(constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]])
{
    (*spvDescriptorSet0.m_9).baz = 0;
    spvDescriptorSet0.img4.write(float4(1.0, 0.0, 0.0, 1.0), uint2(int2(1)));
    spvDescriptorSet0.img.write(spvDescriptorSet0.img3.read(uint2(int2(0))), uint2(int2(0)));
    uint _39 = atomic_fetch_add_explicit((device atomic_uint*)&spvDescriptorSet0.img2_atomic[spvImage2DAtomicCoord(int2(0), spvDescriptorSet0.img2)], 1u, memory_order_relaxed);
    (*spvDescriptorSet0.m_42).foo += 42;
    uint _55 = atomic_fetch_and_explicit((volatile device atomic_uint*)&(*spvDescriptorSet0.m_42).bar, (*spvDescriptorSet0.m_52).quux, memory_order_relaxed);
}

