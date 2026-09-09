// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -O3  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[arr_f3:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 0 Offset 0
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 Offset 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 2 Offset 52

// CHECK: [[arr_f3]] = OpTypeArray %float %uint_3
// CHECK: %type_buffer0 = OpTypeStruct %float [[arr_f3]] %float
// CHECK-NOT: OpTypeStruct
// CHECK-NOT: OpTypeArray

// Type for `float4 color`
// CHECK: %v4float = OpTypeVector %float 4
// CHECK-NOT: OpTypeVector

// CHECK: %buffer0 = OpVariable %_ptr_Uniform_type_buffer0 Uniform

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  float1x3 foo;                      // Offset:   16 Size:    20 [unused]
  float end;                         // Offset:   36 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
// CHECK: [[foo_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float %buffer0 %uint_1 %uint_0
// CHECK: [[foo_0_value:%[a-zA-Z0-9_]+]] = OpLoad %float [[foo_0]]
// CHECK:                  OpFAdd %float {{%[a-zA-Z0-9_]+}} [[foo_0_value]]

  float1x2 bar = foo;
  color.x += bar._m00;
  return color;
}
