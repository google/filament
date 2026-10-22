#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    const int gl_DeviceIndex = 0;
    out.gl_Position = float4(float(gl_DeviceIndex));
    return out;
}

