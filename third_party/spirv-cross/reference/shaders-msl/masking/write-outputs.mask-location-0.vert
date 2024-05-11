#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 v1 [[user(locn1)]];
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
    float gl_ClipDistance [[clip_distance]] [2];
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
};

static inline __attribute__((always_inline))
void write_in_func(thread float4& v0, thread float4& v1, thread float4& gl_Position, thread float& gl_PointSize, thread float (&gl_ClipDistance)[2])
{
    v0 = float4(1.0);
    v1 = float4(2.0);
    gl_Position = float4(3.0);
    gl_PointSize = 4.0;
    gl_ClipDistance[0] = 1.0;
    gl_ClipDistance[1] = 0.5;
}

vertex main0_out main0()
{
    main0_out out = {};
    float4 v0 = {};
    write_in_func(v0, out.v1, out.gl_Position, out.gl_PointSize, out.gl_ClipDistance);
    out.gl_ClipDistance_0 = out.gl_ClipDistance[0];
    out.gl_ClipDistance_1 = out.gl_ClipDistance[1];
    return out;
}

