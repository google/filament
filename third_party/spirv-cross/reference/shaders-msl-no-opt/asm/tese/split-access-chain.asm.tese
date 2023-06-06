#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float o0 [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 in0 [[attribute(0)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    out.o0 = patchIn.gl_in[0u].in0.z;
    return out;
}

