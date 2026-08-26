// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s -check-prefix=ERROR

// CHECK: define void @main()
// CHECK: %[[H:[^ ]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{[^,]+}}, i32 0)
// CHECK: %[[f:[^ ]+]] = extractvalue %dx.types.CBufRet.f32 %[[H]], 0
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %[[f]])
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %[[f]])
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %[[f]])
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %[[f]])

// ERROR: error: no matching function for call to 'badCall'
// ERROR: note: candidate function not viable: no known conversion from 'A' to 'B' for 1st argument

class A {
  float f;
  int i;
};
class B {
  float f;
  int i;
};

float4 badCall(B data) {
  return (float4)data.f;
}

A g_dnc;

float4  main() : SV_Target {
  A dnc = g_dnc;
  return badCall(dnc);
}
