// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Make sure in out is take as inout.
// CHECK: Input signature:
// CHECK: Name
// CHECK: A
// CHECK: Output signature:
// CHECK: Name
// CHECK: A
// CHECK: SV_Position
// CHECK: loadInput

float4 main(in out float4 a: A) : SV_Position {
  return 1.2;
}