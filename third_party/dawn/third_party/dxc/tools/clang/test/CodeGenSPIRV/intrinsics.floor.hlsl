// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'floor' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[floor_a:%[0-9]+]] = OpExtInst %float [[glsl]] Floor [[a]]
// CHECK-NEXT: OpStore %result [[floor_a]]
  float a;
  result = floor(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[floor_b:%[0-9]+]] = OpExtInst %float [[glsl]] Floor [[b]]
// CHECK-NEXT: OpStore %result [[floor_b]]
  float1 b;
  result = floor(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[floor_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Floor [[c]]
// CHECK-NEXT: OpStore %result3 [[floor_c]]
  float3 c;
  result3 = floor(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[floor_d:%[0-9]+]] = OpExtInst %float [[glsl]] Floor [[d]]
// CHECK-NEXT: OpStore %result [[floor_d]]
  float1x1 d;
  result = floor(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[floor_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Floor [[e]]
// CHECK-NEXT: OpStore %result2 [[floor_e]]
  float1x2 e;
  result2 = floor(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[floor_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Floor [[f]]
// CHECK-NEXT: OpStore %result4 [[floor_f]]
  float4x1 f;
  result4 = floor(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[floor_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Floor [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[floor_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Floor [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[floor_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Floor [[g_row2]]
// CHECK-NEXT: [[floor_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[floor_g_row0]] [[floor_g_row1]] [[floor_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[floor_matrix]]
  float3x2 g;
  result3x2 = floor(g);
}
