#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO1
{
    uint values1[1];
};

struct SSBO0
{
    uint values0[1];
};

static inline __attribute__((always_inline))
void callee2(thread float4& gl_FragCoord, device SSBO1& v_7)
{
    int _31 = int(gl_FragCoord.x);
    v_7.values1[_31]++;
}

static inline __attribute__((always_inline))
void callee(thread float4& gl_FragCoord, device SSBO1& v_7, device SSBO0& v_9)
{
    int _39 = int(gl_FragCoord.x);
    v_9.values0[_39]++;
    callee2(gl_FragCoord, v_7);
}

fragment void main0(device SSBO1& v_7 [[buffer(0), raster_order_group(0)]], device SSBO0& v_9 [[buffer(1)]], float4 gl_FragCoord [[position]])
{
    callee(gl_FragCoord, v_7, v_9);
}

