// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'sinh' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[sinh_a:%[0-9]+]] = OpExtInst %float [[glsl]] Sinh [[a]]
// CHECK-NEXT: OpStore %result [[sinh_a]]
  float a;
  result = sinh(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[sinh_b:%[0-9]+]] = OpExtInst %float [[glsl]] Sinh [[b]]
// CHECK-NEXT: OpStore %result [[sinh_b]]
  float1 b;
  result = sinh(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[sinh_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Sinh [[c]]
// CHECK-NEXT: OpStore %result3 [[sinh_c]]
  float3 c;
  result3 = sinh(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[sinh_d:%[0-9]+]] = OpExtInst %float [[glsl]] Sinh [[d]]
// CHECK-NEXT: OpStore %result [[sinh_d]]
  float1x1 d;
  result = sinh(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[sinh_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Sinh [[e]]
// CHECK-NEXT: OpStore %result2 [[sinh_e]]
  float1x2 e;
  result2 = sinh(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[sinh_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Sinh [[f]]
// CHECK-NEXT: OpStore %result4 [[sinh_f]]
  float4x1 f;
  result4 = sinh(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[sinh_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Sinh [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[sinh_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Sinh [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[sinh_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Sinh [[g_row2]]
// CHECK-NEXT: [[sinh_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[sinh_g_row0]] [[sinh_g_row1]] [[sinh_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[sinh_matrix]]
  float3x2 g;
  result3x2 = sinh(g);
}
