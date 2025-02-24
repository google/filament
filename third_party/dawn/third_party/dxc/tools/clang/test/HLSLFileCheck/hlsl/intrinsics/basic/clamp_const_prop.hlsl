// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float -1.250000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 3.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 2.000000e+00)

[RootSignature("")]
float4 main() : SV_Target {
  return float4(
    clamp(10, 0, 1),
    clamp(-1.0f, -2.5f, -1.25f),
    clamp((double)3, (double)-2, (double)5),
    clamp(-5LL, 2LL, 5LL));
}
