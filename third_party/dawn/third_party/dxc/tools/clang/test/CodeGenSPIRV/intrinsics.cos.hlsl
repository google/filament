// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'cos' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[cos_a:%[0-9]+]] = OpExtInst %float [[glsl]] Cos [[a]]
// CHECK-NEXT: OpStore %result [[cos_a]]
  float a;
  result = cos(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[cos_b:%[0-9]+]] = OpExtInst %float [[glsl]] Cos [[b]]
// CHECK-NEXT: OpStore %result [[cos_b]]
  float1 b;
  result = cos(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[cos_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Cos [[c]]
// CHECK-NEXT: OpStore %result3 [[cos_c]]
  float3 c;
  result3 = cos(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[cos_d:%[0-9]+]] = OpExtInst %float [[glsl]] Cos [[d]]
// CHECK-NEXT: OpStore %result [[cos_d]]
  float1x1 d;
  result = cos(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[cos_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Cos [[e]]
// CHECK-NEXT: OpStore %result2 [[cos_e]]
  float1x2 e;
  result2 = cos(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[cos_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Cos [[f]]
// CHECK-NEXT: OpStore %result4 [[cos_f]]
  float4x1 f;
  result4 = cos(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[cos_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Cos [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[cos_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Cos [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[cos_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Cos [[g_row2]]
// CHECK-NEXT: [[cos_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[cos_g_row0]] [[cos_g_row1]] [[cos_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[cos_matrix]]
  float3x2 g;
  result3x2 = cos(g);
}
