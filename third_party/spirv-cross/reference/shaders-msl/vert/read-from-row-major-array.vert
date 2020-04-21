#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float3x4 var[3][4];
};

struct main0_out
{
    float v_vtxResult [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 a_position [[attribute(0)]];
};

static inline __attribute__((always_inline))
float compare_float(thread const float& a, thread const float& b)
{
    return float(abs(a - b) < 0.0500000007450580596923828125);
}

static inline __attribute__((always_inline))
float compare_vec3(thread const float3& a, thread const float3& b)
{
    float param = a.x;
    float param_1 = b.x;
    float param_2 = a.y;
    float param_3 = b.y;
    float param_4 = a.z;
    float param_5 = b.z;
    return (compare_float(param, param_1) * compare_float(param_2, param_3)) * compare_float(param_4, param_5);
}

static inline __attribute__((always_inline))
float compare_mat2x3(thread const float2x3& a, thread const float2x3& b)
{
    float3 param = a[0];
    float3 param_1 = b[0];
    float3 param_2 = a[1];
    float3 param_3 = b[1];
    return compare_vec3(param, param_1) * compare_vec3(param_2, param_3);
}

vertex main0_out main0(main0_in in [[stage_in]], constant Block& _104 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = in.a_position;
    float result = 1.0;
    float2x3 param = transpose(float3x2(_104.var[0][0][0].xy, _104.var[0][0][1].xy, _104.var[0][0][2].xy));
    float2x3 param_1 = float2x3(float3(2.0, 6.0, -6.0), float3(0.0, 5.0, 5.0));
    result *= compare_mat2x3(param, param_1);
    out.v_vtxResult = result;
    return out;
}

