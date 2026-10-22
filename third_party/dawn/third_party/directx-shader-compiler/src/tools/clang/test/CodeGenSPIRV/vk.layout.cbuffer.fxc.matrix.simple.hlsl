// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[arr_f2:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 0 Offset 0
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 Offset 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 2 Offset 36

// CHECK: [[arr_f2]] = OpTypeArray %float %uint_2
// CHECK: %type_buffer0 = OpTypeStruct %float [[arr_f2]] %float
// CHECK: %_ptr_Uniform_type_buffer0 = OpTypePointer Uniform %type_buffer0

// CHECK: [[buffer0_clone:%[a-zA-Z0-9_]+]] = OpTypeStruct %float %v2float %float
// CHECK: [[ptr_buffer0_clone:%[a-zA-Z0-9_]+]] = OpTypePointer Private [[buffer0_clone]]

// CHECK: %buffer0 = OpVariable %_ptr_Uniform_type_buffer0 Uniform
// CHECK:            OpVariable [[ptr_buffer0_clone]] Private

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  float1x2 foo;                      // Offset:   16 Size:    20 [unused]
  float end;                         // Offset:   36 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
  color.x += end;
  return color;
}
