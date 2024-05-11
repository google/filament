#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
    float gl_ClipDistance [[clip_distance]] [2];
};

struct main0_in
{
    float4 pos [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.pos;
    out.gl_ClipDistance[0] = in.pos.x;
    out.gl_ClipDistance[1] = in.pos.y;
    return out;
}

