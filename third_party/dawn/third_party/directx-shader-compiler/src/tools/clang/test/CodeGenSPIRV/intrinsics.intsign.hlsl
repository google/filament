// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'sign' function can operate on int, int vectors, and int matrices.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  int result;
  int3 result3;

// CHECK:      [[a:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[sign_a:%[0-9]+]] = OpExtInst %int [[glsl]] SSign [[a]]
// CHECK-NEXT: OpStore %result [[sign_a]]
  int a;
  result = sign(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[sign_b:%[0-9]+]] = OpExtInst %int [[glsl]] SSign [[b]]
// CHECK-NEXT: OpStore %result [[sign_b]]
  int1 b;
  result = sign(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3int %c
// CHECK-NEXT: [[sign_c:%[0-9]+]] = OpExtInst %v3int [[glsl]] SSign [[c]]
// CHECK-NEXT: OpStore %result3 [[sign_c]]
  int3 c;
  result3 = sign(c);

// CHECK:      [[d:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[sign_d:%[0-9]+]] = OpExtInst %int [[glsl]] SSign [[d]]
// CHECK-NEXT: OpStore %result [[sign_d]]
  int1x1 d;
  result = sign(d);

// CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %v2int %e
// CHECK-NEXT: [[sign_e:%[0-9]+]] = OpExtInst %v2int [[glsl]] SSign [[e]]
// CHECK-NEXT: OpStore %result2 [[sign_e]]
  int1x2 e;
  int2 result2 = sign(e);

// CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %v4int %f
// CHECK-NEXT: [[sign_f:%[0-9]+]] = OpExtInst %v4int [[glsl]] SSign [[f]]
// CHECK-NEXT: OpStore %result4 [[sign_f]]
  int4x1 f;
  int4 result4 = sign(f);

// TODO: Integer matrices are not supported yet. Therefore we cannot run the following test yet.
// XXXXX-NEXT: [[h:%[0-9]+]] = OpLoad %mat3v4int %h
// XXXXX-NEXT: [[h_row0:%[0-9]+]] = OpCompositeExtract %v4int [[h]] 0
// XXXXX-NEXT: [[SSign_h_row0:%[0-9]+]] = OpExtInst %v4int [[glsl]] SSign [[h_row0]]
// XXXXX-NEXT: [[h_row1:%[0-9]+]] = OpCompositeExtract %v4int [[h]] 1
// XXXXX-NEXT: [[SSign_h_row1:%[0-9]+]] = OpExtInst %v4int [[glsl]] SSign [[h_row1]]
// XXXXX-NEXT: [[h_row2:%[0-9]+]] = OpCompositeExtract %v4int [[h]] 2
// XXXXX-NEXT: [[SSign_h_row2:%[0-9]+]] = OpExtInst %v4int [[glsl]] SSign [[h_row2]]
// XXXXX-NEXT: [[SSign_matrix:%[0-9]+]] = OpCompositeConstruct %mat3v4int [[SSign_h_row0]] [[SSign_h_row1]] [[SSign_h_row2]]
// XXXXX-NEXT: OpStore %result3x4 [[SSign_matrix]]
//  int3x4 h;
//  int3x4 result3x4 = sign(h);
}
