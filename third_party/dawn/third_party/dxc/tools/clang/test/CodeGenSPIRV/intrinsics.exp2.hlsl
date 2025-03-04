// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'exp2' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[exp2_a:%[0-9]+]] = OpExtInst %float [[glsl]] Exp2 [[a]]
// CHECK-NEXT: OpStore %result [[exp2_a]]
  float a;
  result = exp2(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[exp2_b:%[0-9]+]] = OpExtInst %float [[glsl]] Exp2 [[b]]
// CHECK-NEXT: OpStore %result [[exp2_b]]
  float1 b;
  result = exp2(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[exp2_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Exp2 [[c]]
// CHECK-NEXT: OpStore %result3 [[exp2_c]]
  float3 c;
  result3 = exp2(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[exp2_d:%[0-9]+]] = OpExtInst %float [[glsl]] Exp2 [[d]]
// CHECK-NEXT: OpStore %result [[exp2_d]]
  float1x1 d;
  result = exp2(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[exp2_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp2 [[e]]
// CHECK-NEXT: OpStore %result2 [[exp2_e]]
  float1x2 e;
  result2 = exp2(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[exp2_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Exp2 [[f]]
// CHECK-NEXT: OpStore %result4 [[exp2_f]]
  float4x1 f;
  result4 = exp2(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[exp2_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp2 [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[exp2_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp2 [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[exp2_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Exp2 [[g_row2]]
// CHECK-NEXT: [[exp2_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[exp2_g_row0]] [[exp2_g_row1]] [[exp2_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[exp2_matrix]]
  float3x2 g;
  result3x2 = exp2(g);
}
