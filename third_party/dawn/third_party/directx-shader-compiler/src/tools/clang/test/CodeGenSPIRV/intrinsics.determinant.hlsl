// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'determinant' function can only operate matrix of floats. The matrix must also be a square matrix.
// The return type is a float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;

// CHECK:      [[a:%[0-9]+]] = OpLoad %mat3v3float %a
// CHECK-NEXT: [[determinant_a:%[0-9]+]] = OpExtInst %float [[glsl]] Determinant [[a]]
// CHECK-NEXT: OpStore %result [[determinant_a]]
  float3x3 a;
  result = determinant(a);
}
