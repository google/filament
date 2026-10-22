#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct t_6
{
    float2x4 m_m0;
    float4 m_m1;
    float m_m2;
    float m_m3;
    float m_m4;
    float m_m5;
};

struct t_7_8
{
    t_6 m_m0;
};

struct t_6_1
{
    float2x4 m_m0;
    float4 m_m1;
    float m_m2;
    float m_m3;
    float m_m4;
    float m_m5;
};

struct t_19
{
    t_6_1 m_m0;
};

struct main0_out
{
    float4 v_3 [[color(0)]];
};

struct main0_in
{
    float4 v_4 [[user(locn1)]];
    float4 v_5_m_m0_m_m0_0 [[user(locn2)]];
    float4 v_5_m_m0_m_m0_1 [[user(locn3)]];
    float4 v_5_m_m0_m_m1 [[user(locn4)]];
    float v_5_m_m0_m_m2 [[user(locn5)]];
    float v_5_m_m0_m_m3 [[user(locn6)]];
    float v_5_m_m0_m_m4 [[user(locn7)]];
    float v_5_m_m0_m_m5 [[user(locn8)]];
};

fragment main0_out main0(main0_in in [[stage_in]], device t_7_8& v_8 [[buffer(0)]])
{
    main0_out out = {};
    t_19 v_5 = {};
    v_5.m_m0.m_m0[0] = in.v_5_m_m0_m_m0_0;
    v_5.m_m0.m_m0[1] = in.v_5_m_m0_m_m0_1;
    v_5.m_m0.m_m1 = in.v_5_m_m0_m_m1;
    v_5.m_m0.m_m2 = in.v_5_m_m0_m_m2;
    v_5.m_m0.m_m3 = in.v_5_m_m0_m_m3;
    v_5.m_m0.m_m4 = in.v_5_m_m0_m_m4;
    v_5.m_m0.m_m5 = in.v_5_m_m0_m_m5;
    out.v_3 = in.v_4;
    v_8.m_m0.m_m0 = v_5.m_m0.m_m0;
    v_8.m_m0.m_m1 = v_5.m_m0.m_m1;
    v_8.m_m0.m_m2 = v_5.m_m0.m_m2;
    v_8.m_m0.m_m3 = v_5.m_m0.m_m3;
    v_8.m_m0.m_m4 = v_5.m_m0.m_m4;
    v_8.m_m0.m_m5 = v_5.m_m0.m_m5;
    return out;
}

