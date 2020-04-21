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

fragment void main0(device Buffer3& _9 [[buffer(0)]], volatile device Buffer& _34 [[buffer(1), raster_order_group(0)]], device Buffer2& _44 [[buffer(2), raster_order_group(0)]], texture2d<float, access::write> img4 [[texture(0)]], texture2d<float, access::write> img [[texture(1), raster_order_group(0)]], texture2d<float> img3 [[texture(2), raster_order_group(0)]])
{
    _9.baz = 0;
    img4.write(float4(1.0, 0.0, 0.0, 1.0), uint2(int2(1)));
    img.write(img3.read(uint2(int2(0))), uint2(int2(0)));
    _34.foo += 42;
    uint _49 = atomic_fetch_and_explicit((volatile device atomic_uint*)&_34.bar, _44.quux, memory_order_relaxed);
}

