#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_PushConstant_Matrix
{
    float4x4 transform;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 in_var_POSITION [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant type_PushConstant_Matrix& matrix_constants [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = matrix_constants.transform * in.in_var_POSITION;
    return out;
}

