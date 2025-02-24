// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'radians' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[radians_a:%[0-9]+]] = OpExtInst %float [[glsl]] Radians [[a]]
// CHECK-NEXT: OpStore %result [[radians_a]]
  float a;
  result = radians(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[radians_b:%[0-9]+]] = OpExtInst %float [[glsl]] Radians [[b]]
// CHECK-NEXT: OpStore %result [[radians_b]]
  float1 b;
  result = radians(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[radians_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Radians [[c]]
// CHECK-NEXT: OpStore %result3 [[radians_c]]
  float3 c;
  result3 = radians(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[radians_d:%[0-9]+]] = OpExtInst %float [[glsl]] Radians [[d]]
// CHECK-NEXT: OpStore %result [[radians_d]]
  float1x1 d;
  result = radians(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[radians_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Radians [[e]]
// CHECK-NEXT: OpStore %result2 [[radians_e]]
  float1x2 e;
  result2 = radians(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[radians_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Radians [[f]]
// CHECK-NEXT: OpStore %result4 [[radians_f]]
  float4x1 f;
  result4 = radians(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[radians_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Radians [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[radians_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Radians [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[radians_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Radians [[g_row2]]
// CHECK-NEXT: [[radians_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[radians_g_row0]] [[radians_g_row1]] [[radians_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[radians_matrix]]
  float3x2 g;
  result3x2 = radians(g);
}
