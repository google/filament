#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

float4 load_subpasses(thread const texture2d<float> uInput, thread float4& gl_FragCoord)
{
    return uInput.read(uint2(gl_FragCoord.xy), 0);
}

fragment main0_out main0(texture2d<float> uSubpass0 [[texture(0)]], texture2d<float> uSubpass1 [[texture(1)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out.FragColor = uSubpass0.read(uint2(gl_FragCoord.xy), 0) + load_subpasses(uSubpass1, gl_FragCoord);
    return out;
}

