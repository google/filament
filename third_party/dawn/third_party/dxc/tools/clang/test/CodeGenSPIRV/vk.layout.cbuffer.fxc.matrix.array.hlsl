// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[type_of_foo:%[a-zA-Z0-9_]+]] ArrayStride 32
// CHECK: OpDecorate [[type_of_bar_elem:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpDecorate [[type_of_bar:%[a-zA-Z0-9_]+]] ArrayStride 48

// CHECK: OpMemberDecorate %type_buffer0 0 Offset 0
// CHECK: OpMemberDecorate %type_buffer0 1 Offset 16
// CHECK: OpMemberDecorate %type_buffer0 1 MatrixStride 16
// CHECK: OpMemberDecorate %type_buffer0 1 RowMajor
// CHECK: OpMemberDecorate %type_buffer0 2 Offset 240
// CHECK: OpMemberDecorate %type_buffer0 3 Offset 468

// CHECK: %mat3v2float = OpTypeMatrix %v2float 3
// CHECK: [[type_of_foo]] = OpTypeArray %mat3v2float %uint_7
// CHECK: [[type_of_bar_elem]] = OpTypeArray %float %uint_3
// CHECK: [[type_of_bar]] = OpTypeArray [[type_of_bar_elem]] %uint_5
// CHECK: %type_buffer0 = OpTypeStruct %float [[type_of_foo]] [[type_of_bar]] %float

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  float3x2 foo[7];                   // Offset:   16 Size:   220 [unused]
  float1x3 bar[5];                   // Offset:  240 Size:   228 [unused]
  float end;                         // Offset:  468 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
  color.x += end;
  return color;
}
