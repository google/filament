// RUN: %dxc -T ps_6_2 -E main -fspv-target-env=vulkan1.1 -enable-16bit-types -fcgl  %s -spirv | FileCheck %s
 
Texture2D tex;
SamplerState texSampler;

cbuffer CBuf {
  float4 avgLum;
  float3x3 someMat;
}

half4 main(float2 uv : UV) : SV_TARGET {
  half4 result = tex.Sample(texSampler, uv);

// Testing that compound multiply-assign works correctly if there are type
// mismatches.
//
// Note: Due to the semantics of operators, the operands are promoted, the
// arithmetic performed, an implicit conversion back to the result type done,
// then the assignment takes place.

// CHECK:            [[avgLum:%[0-9]+]] = OpLoad %float {{%[0-9]+}}
// CHECK:        [[multiplier:%[0-9]+]] = OpFMul %float [[avgLum]] %float_10
// CHECK:            [[result:%[0-9]+]] = OpLoad %v4half %result
// CHECK:        [[result_v4f:%[0-9]+]] = OpFConvert %v4float [[result]]
// CHECK:    [[mul_result_v4f:%[0-9]+]] = OpVectorTimesScalar %v4float [[result_v4f]] [[multiplier]]
// CHECK: [[mul_result_v4half:%[0-9]+]] = OpFConvert %v4half [[mul_result_v4f]]
// CHECK:                              OpStore %result [[mul_result_v4half]]
  result *= avgLum.x * 10.0f;

  half3x3 mat = someMat;
// CHECK:       [[n1_float:%[0-9]+]] = OpFNegate %float %float_1
// CHECK:            [[mat:%[0-9]+]] = OpLoad %mat3v3half %mat
// CHECK:       [[mat_row0:%[0-9]+]] = OpCompositeExtract %v3half [[mat]] 0
// CHECK: [[mat_row0_float:%[0-9]+]] = OpFConvert %v3float [[mat_row0]]
// CHECK:       [[mat_row1:%[0-9]+]] = OpCompositeExtract %v3half [[mat]] 1
// CHECK: [[mat_row1_float:%[0-9]+]] = OpFConvert %v3float [[mat_row1]]
// CHECK:       [[mat_row2:%[0-9]+]] = OpCompositeExtract %v3half [[mat]] 2
// CHECK: [[mat_row2_float:%[0-9]+]] = OpFConvert %v3float [[mat_row2]]
// CHECK:      [[mat_float:%[0-9]+]] = OpCompositeConstruct %mat3v3float [[mat_row0_float]] [[mat_row1_float]] [[mat_row2_float]]
// CHECK:      [[mul_float:%[0-9]+]] = OpMatrixTimesScalar %mat3v3float [[mat_float]] [[n1_float]]
// CHECK: [[mul_row0_float:%[0-9]+]] = OpCompositeExtract %v3float [[mul_float]] 0
// CHECK:  [[mul_row0_half:%[0-9]+]] = OpFConvert %v3half [[mul_row0_float]]
// CHECK: [[mul_row1_float:%[0-9]+]] = OpCompositeExtract %v3float [[mul_float]] 1
// CHECK:  [[mul_row1_half:%[0-9]+]] = OpFConvert %v3half [[mul_row1_float]]
// CHECK: [[mul_row2_float:%[0-9]+]] = OpCompositeExtract %v3float [[mul_float]] 2
// CHECK:  [[mul_row2_half:%[0-9]+]] = OpFConvert %v3half [[mul_row2_float]]
// CHECK:       [[mul_half:%[0-9]+]] = OpCompositeConstruct %mat3v3half [[mul_row0_half]] [[mul_row1_half]] [[mul_row2_half]]
// CHECK:                           OpStore %mat [[mul_half]]
  mat *= -1.f;

  result.xyz = mul(result.xyz, mat);
  return result;
}
