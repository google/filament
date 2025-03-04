// RUN: %dxc -E main -T ps_6_4 -enable-16bit-types -spirv -O0 %s 2>&1 | FileCheck %s

float main() : SV_Target {
// CHECK: warning: conversion from larger type 'float' to smaller type 'vector<half, 2>', possible loss of data
// CHECK: warning: conversion from larger type 'int4' to smaller type 'vector<half, 2>', possible loss of data
// CHECK: warning: implicit truncation of vector type
  float res = 0;

  half2 input1A;
  half2 input1B;
  float acc1;
// CHECK: [[input1A:%[0-9]+]] = OpLoad %v2half %input1A
// CHECK: [[input1B:%[0-9]+]] = OpLoad %v2half %input1B
// CHECK: [[acc1:%[0-9]+]] = OpLoad %float %acc1
// CHECK: [[dot1_0:%[0-9]+]] = OpDot %half [[input1A]] [[input1B]]
// CHECK: [[dot1:%[0-9]+]] = OpFConvert %float [[dot1_0]]
// CHECK: [[res1:%[0-9]+]] = OpFAdd %float [[dot1]] [[acc1]]
  res += dot2add(input1A, input1B, acc1);

  half4 input2;
  int acc2;
// CHECK: [[input2:%[0-9]+]] = OpLoad %v4half %input2
// CHECK: [[input2A:%[0-9]+]] = OpVectorShuffle %v2half [[input2]] [[input2]] 0 1
// CHECK: [[input2_1:%[0-9]+]] = OpLoad %v4half %input2
// CHECK: [[input2B:%[0-9]+]] = OpVectorShuffle %v2half [[input2_1]] [[input2_1]] 2 3
// CHECK: [[acc2_0:%[0-9]+]] = OpLoad %int %acc2
// CHECK: [[acc2:%[0-9]+]] = OpConvertSToF %float [[acc2_0]]
// CHECK: [[dot2_0:%[0-9]+]] = OpDot %half [[input2A]] [[input2B]]
// CHECK: [[dot2:%[0-9]+]] = OpFConvert %float [[dot2_0]]
// CHECK: [[res2:%[0-9]+]] = OpFAdd %float [[dot2]] [[acc2]]
  res += dot2add(input2.xy, input2.zw, acc2);

  float input3A;
  int4 input3B;
  half acc3;
// CHECK: [[input3A_1:%[0-9]+]] = OpLoad %float %input3A
// CHECK: [[input3A_2:%[0-9]+]] = OpFConvert %half [[input3A_1]]
// CHECK: [[input3A:%[0-9]+]] = OpCompositeConstruct %v2half [[input3A_2]] [[input3A_2]]
// CHECK: [[input3B_1:%[0-9]+]] = OpLoad %v4int %input3B
// CHECK: [[input3B_2:%[0-9]+]] = OpCompositeExtract %int [[input3B_1]] 0
// CHECK: [[input3B_3:%[0-9]+]] = OpCompositeExtract %int [[input3B_1]] 1
// CHECK: [[input3B_4:%[0-9]+]] = OpCompositeConstruct %v2int [[input3B_2]] [[input3B_3]]
// CHECK: [[input3B_5:%[0-9]+]] = OpSConvert %v2short [[input3B_4]]
// CHECK: [[input3B:%[0-9]+]] = OpConvertSToF %v2half [[input3B_5]]
// CHECK: [[acc3_1:%[0-9]+]] = OpLoad %half %acc3
// CHECK: [[acc3:%[0-9]+]] = OpFConvert %float [[acc3_1]]
// CHECK: [[dot3_1:%[0-9]+]] = OpDot %half [[input3A]] [[input3B]]
// CHECK: [[dot3:%[0-9]+]] = OpFConvert %float [[dot3_1]]
// CHECK: [[res3:%[0-9]+]] = OpFAdd %float [[dot3]] [[acc3]]
  res += dot2add(input3A, input3B, acc3);

  return res;
}
