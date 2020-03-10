#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float4 load_subpasses(thread const texture2d_ms<float> uInput, thread uint& gl_SampleID, thread float4& gl_FragCoord)
{
    return uInput.read(uint2(gl_FragCoord.xy), gl_SampleID);
}

fragment main0_out main0(texture2d_ms<float> uSubpass0 [[texture(0)]], texture2d_ms<float> uSubpass1 [[texture(1)]], uint gl_SampleID [[sample_id]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out.FragColor = (uSubpass0.read(uint2(gl_FragCoord.xy), 1) + uSubpass1.read(uint2(gl_FragCoord.xy), 2)) + load_subpasses(uSubpass0, gl_SampleID, gl_FragCoord);
    return out;
}

