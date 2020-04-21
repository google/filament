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
void callee2(device SSBO1& v_14, thread float4& gl_FragCoord)
{
    int _25 = int(gl_FragCoord.x);
    v_14.values1[_25]++;
}

static inline __attribute__((always_inline))
void callee(device SSBO1& v_14, thread float4& gl_FragCoord, device SSBO0& v_35)
{
    int _38 = int(gl_FragCoord.x);
    v_35.values0[_38]++;
    callee2(v_14, gl_FragCoord);
}

fragment void main0(device SSBO1& v_14 [[buffer(0), raster_order_group(0)]], device SSBO0& v_35 [[buffer(1), raster_order_group(0)]], float4 gl_FragCoord [[position]])
{
    callee(v_14, gl_FragCoord, v_35);
}

