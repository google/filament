#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o0 [[color(0)]];
};

struct main0_in
{
    float2 m_2 [[user(locn1)]];
    float m_3 [[user(locn1_2)]];
    float m_4 [[user(locn2), flat]];
    uint m_5 [[user(locn2_1)]];
    uint m_6 [[user(locn2_2)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 v1;
    v1 = float4(in.m_2.x, in.m_2.y, v1.z, v1.w);
    v1.z = in.m_3;
    float4 v2;
    v2.x = in.m_4;
    v2.y = as_type<float>(in.m_5);
    v2.z = as_type<float>(in.m_6);
    float4 r0;
    r0.x = as_type<float>(as_type<int>(v2.y) + as_type<int>(v2.z));
    out.o0.y = float(as_type<uint>(r0.x));
    out.o0.x = v1.y + v2.x;
    out.o0 = float4(out.o0.x, out.o0.y, v1.z, v1.x);
    return out;
}

