#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO1
{
    uint values1[1];
};

struct _12
{
    uint _m0[1];
};

struct SSBO0
{
    uint values0[1];
};

static inline __attribute__((always_inline))
void callee2(thread float4& gl_FragCoord, device SSBO1& _7)
{
    int _44 = int(gl_FragCoord.x);
    _7.values1[_44]++;
}

static inline __attribute__((always_inline))
void callee(thread float4& gl_FragCoord, device SSBO1& _7, device SSBO0& _9)
{
    int _52 = int(gl_FragCoord.x);
    _9.values0[_52]++;
    callee2(gl_FragCoord, _7);
    if (true)
    {
    }
}

static inline __attribute__((always_inline))
void _35(thread float4& gl_FragCoord, device _12& _13)
{
    _13._m0[int(gl_FragCoord.x)] = 4u;
}

fragment void main0(device SSBO1& _7 [[buffer(0), raster_order_group(0)]], device _12& _13 [[buffer(1)]], device SSBO0& _9 [[buffer(2), raster_order_group(0)]], float4 gl_FragCoord [[position]])
{
    callee(gl_FragCoord, _7, _9);
    _35(gl_FragCoord, _13);
}

