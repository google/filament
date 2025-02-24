// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure implict cast on compound assign works.
// CHECK: fptoui float {{.*}} to i32
// CHECK: uitofp i32 {{.*}} to float
// CHECK: fadd
// CHECK: fptoui float {{.*}} to i32
// CHECK: uitofp i32 {{.*}} to float

float main(float4 a : A, float b : B) : SV_Target {
  uint c = b;
  c += a;
  return c;
}
