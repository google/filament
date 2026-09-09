// RUN: %dxc -E main -T ps_6_2 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// This is a regression test for github issue #3041.

// CHECK: define void @main()


static const float sFoo = 1.5;

static const float3x3 sBar = half3x3
(
    sFoo,  0.0f,   -sFoo * 0.5f,
    0.0f,   sFoo,  -sFoo * 0.5f,
    0.0f,   0.0f,   1.0f
);

half4 main() : SV_Target
{
    half3 result = half3(0.0, 0.0, 0.0);
    return(float4(result, 0));
}