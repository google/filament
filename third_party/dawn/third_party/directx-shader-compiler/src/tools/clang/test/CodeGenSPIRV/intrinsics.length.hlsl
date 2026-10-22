// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'length' function can only operate on vector of floats.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[length_a:%[0-9]+]] = OpExtInst %float [[glsl]] Length [[a]]
// CHECK-NEXT: OpStore %result [[length_a]]
  float a;
  result = length(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[length_b:%[0-9]+]] = OpExtInst %float [[glsl]] Length [[b]]
// CHECK-NEXT: OpStore %result [[length_b]]
  float1 b;
  result = length(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[length_c:%[0-9]+]] = OpExtInst %float [[glsl]] Length [[c]]
// CHECK-NEXT: OpStore %result [[length_c]]
  float3 c;
  result = length(c);
}
