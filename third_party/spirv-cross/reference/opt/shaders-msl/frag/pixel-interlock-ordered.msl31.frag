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

fragment void main0(device Buffer3& _9 [[buffer(0)]], volatile device Buffer& _42 [[buffer(1), raster_order_group(0)]], device Buffer2& _52 [[buffer(2), raster_order_group(0)]], texture2d<float, access::write> img4 [[texture(0)]], texture2d<float, access::write> img [[texture(1), raster_order_group(0)]], texture2d<float> img3 [[texture(2), raster_order_group(0)]], texture2d<uint, access::read_write> img2 [[texture(3), raster_order_group(0)]])
{
    _9.baz = 0;
    img4.write(float4(1.0, 0.0, 0.0, 1.0), uint2(int2(1)));
    img.write(img3.read(uint2(int2(0))), uint2(int2(0)));
    uint _39 = img2.atomic_fetch_add(uint2(int2(0)), 1u).x;
    _42.foo += 42;
    uint _55 = atomic_fetch_and_explicit((volatile device atomic_uint*)&_42.bar, _52.quux, memory_order_relaxed);
}

