// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[arr_f2:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpMemberDecorate %layout 0 Offset 0
// CHECK: OpMemberDecorate %layout 1 Offset 16
// CHECK: OpMemberDecorate %layout 2 Offset 36
// CHECK: OpMemberDecorate %type_buffer0 0 Offset 0
// CHECK: OpMemberDecorate %type_buffer0 1 Offset 16
// CHECK: OpMemberDecorate %type_buffer0 2 Offset 56

// CHECK: [[arr_f2]] = OpTypeArray %float %uint_2
// CHECK: %layout = OpTypeStruct %float [[arr_f2]] %float
// CHECK: %type_buffer0 = OpTypeStruct %float %layout %float
// CHECK: %_ptr_Uniform_type_buffer0 = OpTypePointer Uniform %type_buffer0

// CHECK: [[layout_clone:%[a-zA-Z0-9_]+]] = OpTypeStruct %float %v2float %float
// CHECK: [[type_buffer0_clone:%[a-zA-Z0-9_]+]] = OpTypeStruct %float [[layout_clone]] %float
// CHECK: [[ptr_type_buffer0_clone:%[a-zA-Z0-9_]+]] = OpTypePointer Private [[type_buffer0_clone]]

// CHECK: %buffer0 = OpVariable %_ptr_Uniform_type_buffer0 Uniform
// CHECK: [[buffer0_clone:%[a-zA-Z0-9_]+]] = OpVariable [[ptr_type_buffer0_clone]] Private

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  struct layout
  {
      float1x1 dummy0;               // Offset:   16
      float1x2 foo;                  // Offset:   32
      float end;                     // Offset:   52

  } bar;                             // Offset:   16 Size:    40 [unused]
  float end;                         // Offset:   56 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
// CHECK: %main = OpFunction %void None
// CHECK:         OpFunctionCall %void %module_init
// CHECK:         OpFunctionCall %v4float %src_main

  color.x += end;

  color.x += bar.foo._12;

  return color;
}

// CHECK: %module_init = OpFunction %void
// CHECK: %module_init_bb = OpLabel
// CHECK: [[ptr_layout:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_layout %buffer0 %uint_1
// CHECK: [[ptr_layout_clone:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_layout_0 [[buffer0_clone]] %uint_1

// CHECK: [[ptr_foo:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_2 [[ptr_layout]] %uint_1
// CHECK: [[ptr_foo_clone:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_v2float [[ptr_layout_clone]] %uint_1
// CHECK: [[ptr_foo_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[ptr_foo]] %uint_0
// CHECK: [[ptr_foo_clone_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[ptr_foo_clone]] %uint_0
// CHECK: [[foo_0:%[a-zA-Z0-9_]+]] = OpLoad %float [[ptr_foo_0]]
// CHECK: OpStore [[ptr_foo_clone_0]] [[foo_0]]
// CHECK: [[ptr_foo_1:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[ptr_foo]] %uint_1
// CHECK: [[ptr_foo_clone_1:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[ptr_foo_clone]] %uint_1
// CHECK: [[foo_1:%[a-zA-Z0-9_]+]] = OpLoad %float [[ptr_foo_1]]
// CHECK: OpStore [[ptr_foo_clone_1]] [[foo_1]]
