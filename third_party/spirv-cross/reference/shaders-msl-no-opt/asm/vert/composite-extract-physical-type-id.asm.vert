#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Float2Array
{
    float4 arr[3];
};

struct main0_out
{
    float4 gl_Position [[position]];
};

static inline __attribute__((always_inline))
float4 src_VSMain(thread const uint& i, constant type_Float2Array& Float2Array)
{
    return float4(Float2Array.arr[i].x, Float2Array.arr[i].y, 0.0, 1.0);
}

vertex main0_out main0(constant type_Float2Array& Float2Array [[buffer(0)]], uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    uint param_var_i = gl_VertexIndex;
    out.gl_Position = src_VSMain(param_var_i, Float2Array);
    return out;
}

