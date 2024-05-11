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
    return out;
}

