#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o0 [[color(0)]];
};

struct main0_in
{
    float2 m_8 [[user(locn1)]];
    float m_16 [[user(locn1_2)]];
    float m_22 [[user(locn2), flat]];
    uint m_28 [[user(locn2_1)]];
    uint m_33 [[user(locn2_2)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 v1;
    v1 = float4(in.m_8.x, in.m_8.y, v1.z, v1.w);
    v1.z = in.m_16;
    float4 v2;
    v2.x = in.m_22;
    v2.y = as_type<float>(in.m_28);
    v2.z = as_type<float>(in.m_33);
    float4 r0;
    r0.x = as_type<float>(as_type<int>(v2.y) + as_type<int>(v2.z));
    out.o0.y = float(as_type<uint>(r0.x));
    out.o0.x = v1.y + v2.x;
    out.o0 = float4(out.o0.x, out.o0.y, v1.z, v1.x);
    return out;
}

