// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float3x2 result3x2;
  int3 iresult3;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[FAbs_a:%[0-9]+]] = OpExtInst %float [[glsl]] FAbs [[a]]
// CHECK-NEXT: OpStore %result [[FAbs_a]]
  float a;
  result = abs(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[FAbs_b:%[0-9]+]] = OpExtInst %float [[glsl]] FAbs [[b]]
// CHECK-NEXT: OpStore %result [[FAbs_b]]
  float1 b;
  result = abs(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[FAbs_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] FAbs [[c]]
// CHECK-NEXT: OpStore %result3 [[FAbs_c]]
  float3 c;
  result3 = abs(c);

// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[FAbs_d:%[0-9]+]] = OpExtInst %float [[glsl]] FAbs [[d]]
// CHECK-NEXT: OpStore %result [[FAbs_d]]
  float1x1 d;
  result = abs(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: [[FAbs_e:%[0-9]+]] = OpExtInst %v2float [[glsl]] FAbs [[e]]
// CHECK-NEXT: OpStore %result2 [[FAbs_e]]
  float1x2 e;
  result2 = abs(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4float %f
// CHECK-NEXT: [[FAbs_f:%[0-9]+]] = OpExtInst %v4float [[glsl]] FAbs [[f]]
// CHECK-NEXT: OpStore %result4 [[FAbs_f]]
  float4x1 f;
  result4 = abs(f);

// CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %mat3v2float %g
// CHECK-NEXT: [[g_row0:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 0
// CHECK-NEXT: [[FAbs_g_row0:%[0-9]+]] = OpExtInst %v2float [[glsl]] FAbs [[g_row0]]
// CHECK-NEXT: [[g_row1:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 1
// CHECK-NEXT: [[FAbs_g_row1:%[0-9]+]] = OpExtInst %v2float [[glsl]] FAbs [[g_row1]]
// CHECK-NEXT: [[g_row2:%[0-9]+]] = OpCompositeExtract %v2float [[g]] 2
// CHECK-NEXT: [[FAbs_g_row2:%[0-9]+]] = OpExtInst %v2float [[glsl]] FAbs [[g_row2]]
// CHECK-NEXT: [[FAbs_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[FAbs_g_row0]] [[FAbs_g_row1]] [[FAbs_g_row2]]
// CHECK-NEXT: OpStore %result3x2 [[FAbs_matrix]]
  float3x2 g;
  result3x2 = abs(g);

// CHECK-NEXT: [[i:%[0-9]+]] = OpLoad %v3int %i
// CHECK-NEXT: [[SAbs_i:%[0-9]+]] = OpExtInst %v3int [[glsl]] SAbs [[i]]
// CHECK-NEXT: OpStore %iresult3 [[SAbs_i]]
  int3 i;
  iresult3 = abs(i);

// TODO: Integer matrices are not supported yet. Therefore we cannot run the following test yet.
// XXXXX-NEXT: [[h:%[0-9]+]] = OpLoad %mat3v4float %h
// XXXXX-NEXT: [[h_row0:%[0-9]+]] = OpCompositeExtract %v4float [[h]] 0
// XXXXX-NEXT: [[SAbs_h_row0:%[0-9]+]] = OpExtInst %v4float [[glsl]] SAbs [[h_row0]]
// XXXXX-NEXT: [[h_row1:%[0-9]+]] = OpCompositeExtract %v4float [[h]] 1
// XXXXX-NEXT: [[SAbs_h_row1:%[0-9]+]] = OpExtInst %v4float [[glsl]] SAbs [[h_row1]]
// XXXXX-NEXT: [[h_row2:%[0-9]+]] = OpCompositeExtract %v4float [[h]] 2
// XXXXX-NEXT: [[SAbs_h_row2:%[0-9]+]] = OpExtInst %v4float [[glsl]] SAbs [[h_row2]]
// XXXXX-NEXT: [[SAbs_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v4float [[SAbs_h_row0]] [[SAbs_h_row1]] [[SAbs_h_row2]]
// XXXXX-NEXT: OpStore %result3x4 [[SAbs_matrix]]
//  int3x4 h;
//  int3x4 result3x4 = abs(h);
}
