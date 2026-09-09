// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'degrees' function can only operate on float, vector of float, and matrix of float.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[degrees_a:%[0-9]+]] = OpExtInst %float [[glsl]] Degrees [[a]]
// CHECK-NEXT: OpStore %result [[degrees_a]]
  float a;
  result = degrees(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[degrees_b:%[0-9]+]] = OpExtInst %float [[glsl]] Degrees [[b]]
// CHECK-NEXT: OpStore %result [[degrees_b]]
  float1 b;
  result = degrees(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[degrees_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Degrees [[c]]
// CHECK-NEXT: OpStore %result3 [[degrees_c]]
  float3 c;
  result3 = degrees(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[degrees_d:%[0-9]+]] = OpExtInst %float [[glsl]] Degrees [[d]]
// CHECK-NEXT: OpStore %result [[degrees_d]]
  float1x1 d;
  result = degrees(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[degrees_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] Degrees [[e]]
// CHECK-NEXT: OpStore %result2 [[degrees_e]]
  float1x2 e;
  result2 = degrees(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[degrees_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] Degrees [[f]]
// CHECK-NEXT: OpStore %result4 [[degrees_f]]
  float4x1 f;
  result4 = degrees(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[degrees_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] Degrees [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[degrees_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] Degrees [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[degrees_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] Degrees [[g_row2]]
// CHECK-NEXT: [[degrees_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[degrees_g_row0]] [[degrees_g_row1]] [[degrees_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[degrees_matrix]]
  float3x2 g;
  result3x2 = degrees(g);
}
