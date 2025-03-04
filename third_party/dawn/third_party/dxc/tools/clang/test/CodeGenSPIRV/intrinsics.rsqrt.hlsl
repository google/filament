// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'rsqrt' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[rsqrt_a:%[0-9]+]] = OpExtInst %float [[glsl]] InverseSqrt [[a]]
// CHECK-NEXT: OpStore %result [[rsqrt_a]]
  float a;
  result = rsqrt(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[rsqrt_b:%[0-9]+]] = OpExtInst %float [[glsl]] InverseSqrt [[b]]
// CHECK-NEXT: OpStore %result [[rsqrt_b]]
  float1 b;
  result = rsqrt(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[rsqrt_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] InverseSqrt [[c]]
// CHECK-NEXT: OpStore %result3 [[rsqrt_c]]
  float3 c;
  result3 = rsqrt(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[rsqrt_d:%[0-9]+]] = OpExtInst %float [[glsl]] InverseSqrt [[d]]
// CHECK-NEXT: OpStore %result [[rsqrt_d]]
  float1x1 d;
  result = rsqrt(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[rsqrt_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] InverseSqrt [[e]]
// CHECK-NEXT: OpStore %result2 [[rsqrt_e]]
  float1x2 e;
  result2 = rsqrt(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[rsqrt_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] InverseSqrt [[f]]
// CHECK-NEXT: OpStore %result4 [[rsqrt_f]]
  float4x1 f;
  result4 = rsqrt(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[rsqrt_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] InverseSqrt [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[rsqrt_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] InverseSqrt [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[rsqrt_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] InverseSqrt [[g_row2]]
// CHECK-NEXT: [[rsqrt_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[rsqrt_g_row0]] [[rsqrt_g_row1]] [[rsqrt_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[rsqrt_matrix]]
  float3x2 g;
  result3x2 = rsqrt(g);
}
