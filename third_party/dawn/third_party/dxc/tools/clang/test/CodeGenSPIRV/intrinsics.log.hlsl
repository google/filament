// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'log' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[log_a:%[0-9]+]] = OpExtInst %float [[glsl]] Log [[a]]
// CHECK-NEXT: OpStore %result [[log_a]]
  float a;
  result = log(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[log_b:%[0-9]+]] = OpExtInst %float [[glsl]] Log [[b]]
// CHECK-NEXT: OpStore %result [[log_b]]
  float1 b;
  result = log(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[log_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Log [[c]]
// CHECK-NEXT: OpStore %result3 [[log_c]]
  float3 c;
  result3 = log(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[log_d:%[0-9]+]] = OpExtInst %float [[glsl]] Log [[d]]
// CHECK-NEXT: OpStore %result [[log_d]]
  float1x1 d;
  result = log(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[log_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Log [[e]]
// CHECK-NEXT: OpStore %result2 [[log_e]]
  float1x2 e;
  result2 = log(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[log_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Log [[f]]
// CHECK-NEXT: OpStore %result4 [[log_f]]
  float4x1 f;
  result4 = log(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[log_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Log [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[log_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Log [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[log_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Log [[g_row2]]
// CHECK-NEXT: [[log_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[log_g_row0]] [[log_g_row1]] [[log_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[log_matrix]]
  float3x2 g;
  result3x2 = log(g);
}
