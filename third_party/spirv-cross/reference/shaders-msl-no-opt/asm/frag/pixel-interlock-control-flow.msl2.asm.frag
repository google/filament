#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO1
{
    uint values1[1];
};

struct _16
{
    uint _m0[1];
};

struct SSBO0
{
    uint values0[1];
};

static inline __attribute__((always_inline))
void callee2(thread float4& gl_FragCoord, device SSBO1& _11)
{
    int _25 = int(gl_FragCoord.x);
    _11.values1[_25]++;
}

static inline __attribute__((always_inline))
void callee(thread float4& gl_FragCoord, device SSBO1& _11, device SSBO0& _13)
{
    int _38 = int(gl_FragCoord.x);
    _13.values0[_38]++;
    callee2(gl_FragCoord, _11);
    if (true)
    {
    }
}

static inline __attribute__((always_inline))
void _52(thread float4& gl_FragCoord, device _16& _17)
{
    _17._m0[int(gl_FragCoord.x)] = 4u;
}

fragment void main0(device SSBO1& _11 [[buffer(0), raster_order_group(0)]], device _16& _17 [[buffer(1)]], device SSBO0& _13 [[buffer(2), raster_order_group(0)]], float4 gl_FragCoord [[position]])
{
    callee(gl_FragCoord, _11, _13);
    _52(gl_FragCoord, _17);
}

