// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'round' function can only operate on float, vector of float, and matrix of float.
// Rounds the specified value to the nearest integer. Halfway cases are rounded to the nearest even.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[round_a:%[0-9]+]] = OpExtInst %float [[glsl]] RoundEven [[a]]
// CHECK-NEXT: OpStore %result [[round_a]]
  float a;
  result = round(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[round_b:%[0-9]+]] = OpExtInst %float [[glsl]] RoundEven [[b]]
// CHECK-NEXT: OpStore %result [[round_b]]
  float1 b;
  result = round(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[round_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] RoundEven [[c]]
// CHECK-NEXT: OpStore %result3 [[round_c]]
  float3 c;
  result3 = round(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[round_d:%[0-9]+]] = OpExtInst %float [[glsl]] RoundEven [[d]]
// CHECK-NEXT: OpStore %result [[round_d]]
  float1x1 d;
  result = round(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[round_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] RoundEven [[e]]
// CHECK-NEXT: OpStore %result2 [[round_e]]
  float1x2 e;
  result2 = round(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[round_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] RoundEven [[f]]
// CHECK-NEXT: OpStore %result4 [[round_f]]
  float4x1 f;
  result4 = round(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[round_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] RoundEven [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[round_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] RoundEven [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[round_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] RoundEven [[g_row2]]
// CHECK-NEXT: [[round_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[round_g_row0]] [[round_g_row1]] [[round_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[round_matrix]]
  float3x2 g;
  result3x2 = round(g);
}
