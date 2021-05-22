#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    half4 out_var_SV_Target [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 in_var_POSITION [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = float4(in.in_var_POSITION, 0.0, 1.0);
    return out;
}

