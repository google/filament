// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'acos' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[acos_a:%[0-9]+]] = OpExtInst %float [[glsl]] Acos [[a]]
// CHECK-NEXT: OpStore %result [[acos_a]]
  float a;
  result = acos(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[acos_b:%[0-9]+]] = OpExtInst %float [[glsl]] Acos [[b]]
// CHECK-NEXT: OpStore %result [[acos_b]]
  float1 b;
  result = acos(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[acos_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Acos [[c]]
// CHECK-NEXT: OpStore %result3 [[acos_c]]
  float3 c;
  result3 = acos(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[acos_d:%[0-9]+]] = OpExtInst %float [[glsl]] Acos [[d]]
// CHECK-NEXT: OpStore %result [[acos_d]]
  float1x1 d;
  result = acos(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[acos_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Acos [[e]]
// CHECK-NEXT: OpStore %result2 [[acos_e]]
  float1x2 e;
  result2 = acos(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[acos_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Acos [[f]]
// CHECK-NEXT: OpStore %result4 [[acos_f]]
  float4x1 f;
  result4 = acos(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[acos_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Acos [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[acos_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Acos [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[acos_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Acos [[g_row2]]
// CHECK-NEXT: [[acos_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[acos_g_row0]] [[acos_g_row1]] [[acos_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[acos_matrix]]
  float3x2 g;
  result3x2 = acos(g);
}
