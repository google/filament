#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct params
{
    float4x4 mvp;
    float psize;
};

struct main0_out
{
    float4 color [[user(locn0)]];
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
};

struct main0_in
{
    float4 position [[attribute(0)]];
    float4 color0 [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant params& _19 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _19.mvp * in.position;
    out.gl_PointSize = _19.psize;
    out.color = in.color0;
    return out;
}

