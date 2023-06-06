#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct MVPs
{
    float4x4 MVP[2];
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant uint* spvViewMask [[buffer(24)]], constant MVPs& _19 [[buffer(0)]])
{
    main0_out out = {};
    const uint gl_ViewIndex = spvViewMask[0];
    out.gl_Position = _19.MVP[int(gl_ViewIndex)] * in.Position;
    return out;
}

