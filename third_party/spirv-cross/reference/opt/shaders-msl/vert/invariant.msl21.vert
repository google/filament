#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position, invariant]];
};

struct main0_in
{
    float4 vInput0 [[attribute(0)]];
    float4 vInput1 [[attribute(1)]];
    float4 vInput2 [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 _20 = in.vInput1 * in.vInput2;
    float4 _21 = in.vInput0 + _20;
    out.gl_Position = _21;
    return out;
}

