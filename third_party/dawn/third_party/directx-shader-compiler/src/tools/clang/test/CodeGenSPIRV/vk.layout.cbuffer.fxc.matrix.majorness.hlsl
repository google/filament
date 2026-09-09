// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[type_of_foo:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpDecorate %_arr_mat2v3float_uint_7 ArrayStride 48
// CHECK: OpDecorate %_arr_float_uint_3 ArrayStride 16
// CHECK: OpDecorate %_arr__arr_float_uint_3_uint_5 ArrayStride 48
// CHECK: OpMemberDecorate %type_buffer0 0 Offset 0
// CHECK: OpMemberDecorate %type_buffer0 1 Offset 16
// CHECK: OpMemberDecorate %type_buffer0 2 Offset 48
// CHECK: OpMemberDecorate %type_buffer0 2 MatrixStride 16
// CHECK: OpMemberDecorate %type_buffer0 2 RowMajor
// CHECK: OpMemberDecorate %type_buffer0 3 Offset 384
// CHECK: OpMemberDecorate %type_buffer0 4 Offset 624
// CHECK: OpMemberDecorate %type_buffer0 5 Offset 852

// CHECK: [[type_of_foo]] = OpTypeArray %float %uint_2
// CHECK: %_arr_float_uint_3 = OpTypeArray %float %uint_3
// CHECK: %_arr__arr_float_uint_3_uint_5 = OpTypeArray %_arr_float_uint_3 %uint_5
// CHECK: %type_buffer0 = OpTypeStruct %float [[type_of_foo]] %_arr_mat2v3float_uint_7 %_arr__arr_float_uint_3_uint_5 %_arr__arr_float_uint_3_uint_5 %float

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  float1x2 foo;                      // Offset:   16 Size:    20 [unused]
  float2x3 bar[7];                   // Offset:   48 Size:   328 [unused]
  row_major float3x1 zar[5];         // Offset:  384 Size:   228 [unused]
  float1x3 x[5];                     // Offset:  624 Size:   228 [unused]
  float end;                         // Offset:  852 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
// CHECK: [[type_of_arr:%[a-zA-Z0-9_]+]] = OpTypeArray %float %uint_2
// CHECK: [[ptr_type_of_arr:%[a-zA-Z0-9_]+]] = OpTypePointer Function [[type_of_arr]]

// CHECK: %arr = OpVariable [[ptr_type_of_arr]] Function

  float arr[2];
  color.x += end;
  return color;
}
