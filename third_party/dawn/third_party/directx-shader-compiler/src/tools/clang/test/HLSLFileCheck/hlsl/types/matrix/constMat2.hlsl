// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fmul
// CHECK: 4.000000e+00
// CHECK: fmul
// CHECK: 7.000000e+00
// CHECK: @dx.op.storeOutput.f32
static const float3x3 g_mat1 = {
    1, 2, 3,
    4, 5, 6,
    7, 8, 9,
};

float4 main(float a : A) : SV_Target {
    float3 v = float3(a, 0, 0);
    return float4(mul(g_mat1, v), 0);
}