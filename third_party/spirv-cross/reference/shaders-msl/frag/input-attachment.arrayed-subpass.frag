#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float4 load_subpasses(texture2d_array<float> uInput, thread float4& gl_FragCoord, thread uint& gl_Layer)
{
    return uInput.read(uint2(gl_FragCoord.xy), gl_Layer);
}

fragment main0_out main0(texture2d_array<float> uSubpass0 [[texture(0)]], texture2d_array<float> uSubpass1 [[texture(1)]], float4 gl_FragCoord [[position]], uint gl_Layer [[render_target_array_index]])
{
    main0_out out = {};
    out.FragColor = uSubpass0.read(uint2(gl_FragCoord.xy), gl_Layer) + load_subpasses(uSubpass1, gl_FragCoord, gl_Layer);
    return out;
}

