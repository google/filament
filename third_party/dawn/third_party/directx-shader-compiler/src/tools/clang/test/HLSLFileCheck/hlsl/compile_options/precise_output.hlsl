// RUN: %dxc -E main -T vs_6_0 -precise-output d1 -precise-output SV_Position %s | FileCheck %s

// Make sure get ir.
// CHECK:@main
// Make sure precise removed fast.
// CHECK-NOT:fast

struct T {
  float4 p : SV_Position;
  float a : A;
};

T main(float4 a:A, float b:B, float c:C, out float d:D1) {

   T t;
  t.p = a + b;
  t.a = b;
   d = b*c;
  return t;
}