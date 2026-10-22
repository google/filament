// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure this works on function that returns matrix
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoad

float4x4 m;
float4x4 GetMat() {
  return m;
}

float main() : SV_Target {
  return GetMat()._m01;
}