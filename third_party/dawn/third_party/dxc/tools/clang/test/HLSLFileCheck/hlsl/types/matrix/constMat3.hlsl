// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: [9 x float] [float 1.000000e+00, float 4.000000e+00, float 7.000000e+00, float 2.000000e+00, float 5.000000e+00, float 8.000000e+00, float 3.000000e+00, float 6.000000e+00, float 9.000000e+00]
// CHECK: fmul
// CHECK: 2.000000e+00
// CHECK: fmul
// CHECK: 3.000000e+00
// CHECK: add i32
// CHECK: , 3
// CHECK: add i32
// CHECK: , 6

static const float3x3 g_mat1 = {
    1, 2, 3,
    4, 5, 6,
    7, 8, 9,
};

float4 main(float a : A) : SV_Target {
    float3 v = float3(a, 0, 0);
    float4 c = float4(mul(v, g_mat1), 0);
    return c + float4(g_mat1[a],0);
}