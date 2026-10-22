#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct t_9
{
    float2x4 m_m0;
    float4 m_m1;
    float m_m2;
    float m_m3;
    float m_m4;
    float m_m5;
};

struct t_10
{
    t_9 m_m0;
};

struct main0_out
{
    float4 v_5 [[user(locn1)]];
    float4 v_7_m_m0_m_m0_0 [[user(locn2)]];
    float4 v_7_m_m0_m_m0_1 [[user(locn3)]];
    float4 v_7_m_m0_m_m1 [[user(locn4)]];
    float v_7_m_m0_m_m2 [[user(locn5)]];
    float v_7_m_m0_m_m3 [[user(locn6)]];
    float v_7_m_m0_m_m4 [[user(locn7)]];
    float v_7_m_m0_m_m5 [[user(locn8)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 v_4 [[attribute(0)]];
    float4 v_6 [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    t_10 v_7 = {};
    t_10 v_33 = t_10{ t_9{ float2x4(float4(1.0), float4(1.0)), float4(1.0), 1.0, 1.0, 1.0, 1.0 } };
    v_7 = t_10{ t_9{ float2x4(float4(1.0), float4(1.0)), float4(1.0), 1.0, 1.0, 1.0, 1.0 } };
    out.gl_Position = in.v_4;
    out.v_5 = in.v_6;
    out.v_7_m_m0_m_m0_0 = v_7.m_m0.m_m0[0];
    out.v_7_m_m0_m_m0_1 = v_7.m_m0.m_m0[1];
    out.v_7_m_m0_m_m1 = v_7.m_m0.m_m1;
    out.v_7_m_m0_m_m2 = v_7.m_m0.m_m2;
    out.v_7_m_m0_m_m3 = v_7.m_m0.m_m3;
    out.v_7_m_m0_m_m4 = v_7.m_m0.m_m4;
    out.v_7_m_m0_m_m5 = v_7.m_m0.m_m5;
    return out;
}

