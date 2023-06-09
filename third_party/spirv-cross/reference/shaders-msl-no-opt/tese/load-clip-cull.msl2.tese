#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 gl_Position [[attribute(0)]];
    float gl_ClipDistance_0 [[attribute(1)]];
    float gl_ClipDistance_1 [[attribute(2)]];
    float gl_CullDistance_0 [[attribute(3)]];
    float gl_CullDistance_1 [[attribute(4)]];
    float gl_CullDistance_2 [[attribute(5)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    out.gl_Position.x = patchIn.gl_in[0].gl_ClipDistance_0;
    out.gl_Position.y = patchIn.gl_in[1].gl_CullDistance_0;
    out.gl_Position.z = patchIn.gl_in[0].gl_ClipDistance_1;
    out.gl_Position.w = patchIn.gl_in[1].gl_CullDistance_1;
    out.gl_Position += patchIn.gl_in[0].gl_Position;
    out.gl_Position += patchIn.gl_in[1].gl_Position;
    return out;
}

