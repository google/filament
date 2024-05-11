#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VSOut
{
    float4 pos;
    float2 clip;
};

struct main0_out
{
    float4 gl_Position [[position]];
    float gl_ClipDistance [[clip_distance]] [2];
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
};

struct main0_in
{
    float4 pos [[attribute(0)]];
};

static inline __attribute__((always_inline))
VSOut _main(thread const float4& pos)
{
    VSOut vout;
    vout.pos = pos;
    vout.clip = pos.xy;
    return vout;
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 pos = in.pos;
    float4 param = pos;
    VSOut flattenTemp = _main(param);
    out.gl_Position = flattenTemp.pos;
    out.gl_ClipDistance[0] = flattenTemp.clip.x;
    out.gl_ClipDistance[1] = flattenTemp.clip.y;
    out.gl_ClipDistance_0 = out.gl_ClipDistance[0];
    out.gl_ClipDistance_1 = out.gl_ClipDistance[1];
    return out;
}

