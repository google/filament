// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'exp' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[exp_a:%[0-9]+]] = OpExtInst %float [[glsl]] Exp [[a]]
// CHECK-NEXT: OpStore %result [[exp_a]]
  float a;
  result = exp(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[exp_b:%[0-9]+]] = OpExtInst %float [[glsl]] Exp [[b]]
// CHECK-NEXT: OpStore %result [[exp_b]]
  float1 b;
  result = exp(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[exp_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Exp [[c]]
// CHECK-NEXT: OpStore %result3 [[exp_c]]
  float3 c;
  result3 = exp(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[exp_d:%[0-9]+]] = OpExtInst %float [[glsl]] Exp [[d]]
// CHECK-NEXT: OpStore %result [[exp_d]]
  float1x1 d;
  result = exp(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[exp_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp [[e]]
// CHECK-NEXT: OpStore %result2 [[exp_e]]
  float1x2 e;
  result2 = exp(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[exp_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Exp [[f]]
// CHECK-NEXT: OpStore %result4 [[exp_f]]
  float4x1 f;
  result4 = exp(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[exp_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[exp_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[exp_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp [[g_row2]]
// CHECK-NEXT: [[exp_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[exp_g_row0]] [[exp_g_row1]] [[exp_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[exp_matrix]]
  float3x2 g;
  result3x2 = exp(g);
}
