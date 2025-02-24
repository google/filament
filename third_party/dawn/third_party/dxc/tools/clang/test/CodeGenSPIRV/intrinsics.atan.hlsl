// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'atan' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[atan_a:%[0-9]+]] = OpExtInst %float [[glsl]] Atan [[a]]
// CHECK-NEXT: OpStore %result [[atan_a]]
  float a;
  result = atan(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[atan_b:%[0-9]+]] = OpExtInst %float [[glsl]] Atan [[b]]
// CHECK-NEXT: OpStore %result [[atan_b]]
  float1 b;
  result = atan(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[atan_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Atan [[c]]
// CHECK-NEXT: OpStore %result3 [[atan_c]]
  float3 c;
  result3 = atan(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[atan_d:%[0-9]+]] = OpExtInst %float [[glsl]] Atan [[d]]
// CHECK-NEXT: OpStore %result [[atan_d]]
  float1x1 d;
  result = atan(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[atan_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Atan [[e]]
// CHECK-NEXT: OpStore %result2 [[atan_e]]
  float1x2 e;
  result2 = atan(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[atan_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Atan [[f]]
// CHECK-NEXT: OpStore %result4 [[atan_f]]
  float4x1 f;
  result4 = atan(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[atan_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Atan [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[atan_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Atan [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[atan_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Atan [[g_row2]]
// CHECK-NEXT: [[atan_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[atan_g_row0]] [[atan_g_row1]] [[atan_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[atan_matrix]]
  float3x2 g;
  result3x2 = atan(g);
}
