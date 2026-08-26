// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'trunc' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[trunc_a:%[0-9]+]] = OpExtInst %float [[glsl]] Trunc [[a]]
// CHECK-NEXT: OpStore %result [[trunc_a]]
  float a;
  result = trunc(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[trunc_b:%[0-9]+]] = OpExtInst %float [[glsl]] Trunc [[b]]
// CHECK-NEXT: OpStore %result [[trunc_b]]
  float1 b;
  result = trunc(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[trunc_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Trunc [[c]]
// CHECK-NEXT: OpStore %result3 [[trunc_c]]
  float3 c;
  result3 = trunc(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[trunc_d:%[0-9]+]] = OpExtInst %float [[glsl]] Trunc [[d]]
// CHECK-NEXT: OpStore %result [[trunc_d]]
  float1x1 d;
  result = trunc(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[trunc_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Trunc [[e]]
// CHECK-NEXT: OpStore %result2 [[trunc_e]]
  float1x2 e;
  result2 = trunc(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[trunc_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Trunc [[f]]
// CHECK-NEXT: OpStore %result4 [[trunc_f]]
  float4x1 f;
  result4 = trunc(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[trunc_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Trunc [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[trunc_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Trunc [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[trunc_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Trunc [[g_row2]]
// CHECK-NEXT: [[trunc_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[trunc_g_row0]] [[trunc_g_row1]] [[trunc_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[trunc_matrix]]
  float3x2 g;
  result3x2 = trunc(g);
}
