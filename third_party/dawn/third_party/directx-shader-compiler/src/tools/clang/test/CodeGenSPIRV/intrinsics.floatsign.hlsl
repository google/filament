// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'sign' function can operate on float, float vectors, and float matrices.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  int result;
  int3 result3;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[sign_a:%[0-9]+]] = OpExtInst %float [[glsl]] FSign [[a]]
// CHECK-NEXT: [[sign_a_int:%[0-9]+]] = OpConvertFToS %int [[sign_a]]
// CHECK-NEXT: OpStore %result [[sign_a_int]]
  float a;
  result = sign(a);

// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[sign_b:%[0-9]+]] = OpExtInst %float [[glsl]] FSign [[b]]
// CHECK-NEXT: [[sign_b_int:%[0-9]+]] = OpConvertFToS %int [[sign_b]]
// CHECK-NEXT: OpStore %result [[sign_b_int]]
  float1 b;
  result = sign(b);

// CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %v3float %c
// CHECK-NEXT: [[sign_c:%[0-9]+]] = OpExtInst %v3float [[glsl]] FSign [[c]]
// CHECK-NEXT: [[sign_c_int:%[0-9]+]] = OpConvertFToS %v3int [[sign_c]]
// CHECK-NEXT: OpStore %result3 [[sign_c_int]]
  float3 c;
  result3 = sign(c);

///////////////////////////////////////////////////////////////////////////
// Note: The following do not work because FSign returns a float result, //
// whereas the HLSL 'sign' function returns an integer result.           //
// Therefore we need to cast the FSign result into a matrix of integers. //
// Casting to Matrix of integers is currently not supported.             //
///////////////////////////////////////////////////////////////////////////

//  float1x1 d;
//  result = sign(d);

//  float1x2 e;
//  int2 result2 = sign(e);

//  float4x1 f;
//  int4 result4 = sign(f);

//  float3x2 g;
//  int3x2 result3x2 = sign(g);
}
