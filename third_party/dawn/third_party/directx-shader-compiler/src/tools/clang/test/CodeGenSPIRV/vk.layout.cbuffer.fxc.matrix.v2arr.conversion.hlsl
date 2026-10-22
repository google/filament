// RUN: %dxc -T ps_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[arr_f3:%[a-zA-Z0-9_]+]] ArrayStride 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 0 Offset 0
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 1 Offset 16
// CHECK: OpMemberDecorate {{%[a-zA-Z0-9_]+}} 2 Offset 52

// CHECK: [[arr_f3]] = OpTypeArray %float %uint_3
// CHECK: %type_buffer0 = OpTypeStruct %float [[arr_f3]] %float

cbuffer buffer0 {
  float dummy0;                      // Offset:    0 Size:     4 [unused]
  float1x3 foo;                      // Offset:   16 Size:    20 [unused]
  float end;                         // Offset:   36 Size:     4
};

float4 main(float4 color : COLOR) : SV_TARGET
{
// CHECK: %main = OpFunction %void None
// CHECK:         OpFunctionCall %void %module_init
// CHECK:         OpFunctionCall %v4float %src_main

  float1x2 bar = foo;
  color.x += bar._m00;
  return color;
}

// CHECK: %module_init = OpFunction %void
// CHECK: %module_init_bb = OpLabel
// CHECK: [[dummy0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[buffer0:%[a-zA-Z0-9_]+]] %uint_0
// CHECK: [[dummy0_clone:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[clone:%[a-zA-Z0-9_]+]] %uint_0
// CHECK: [[dummy0_value:%[a-zA-Z0-9_]+]] = OpLoad %float [[dummy0]]
// CHECK:                OpStore [[dummy0_clone]] [[dummy0_value]]
// CHECK: [[foo:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_3 [[buffer0]] %uint_1
// CHECK: [[foo_clone:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_v3float [[clone]] %uint_1
// CHECK: [[foo_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[foo]] %uint_0
// CHECK: [[foo_clone_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[foo_clone]] %uint_0
// CHECK: [[foo_0_value:%[a-zA-Z0-9_]+]] = OpLoad %float [[foo_0]]
// CHECK:                OpStore [[foo_clone_0]] [[foo_0_value]]
// CHECK: [[foo_1:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[foo]] %uint_1
// CHECK: [[foo_clone_1:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[foo_clone]] %uint_1
// CHECK: [[foo_1_value:%[a-zA-Z0-9_]+]] = OpLoad %float [[foo_1]]
// CHECK:                OpStore [[foo_clone_1]] [[foo_1_value]]
// CHECK: [[foo_2:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[foo]] %uint_2
// CHECK: [[foo_clone_2:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[foo_clone]] %uint_2
// CHECK: [[foo_2_value:%[a-zA-Z0-9_]+]] = OpLoad %float [[foo_2]]
// CHECK:                OpStore [[foo_clone_2]] [[foo_2_value]]
// CHECK: [[end:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float [[buffer0]] %uint_2
// CHECK: [[end_clone:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Private_float [[clone]] %uint_2
// CHECK: [[end_value:%[a-zA-Z0-9_]+]] = OpLoad %float [[end]]
// CHECK:                OpStore [[end_clone]] [[end_value]]
// CHECK:                OpReturn
// CHECK:                OpFunctionEnd
