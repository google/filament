// RUN: %dxc -E main -T ps_6_4 -fspv-target-env=vulkan1.1 -fcgl  %s -spirv  2>&1 | FileCheck %s

float2 main(uint4 inputs : Inputs0, uint acc0 : Acc0, int acc1 : Acc1) : SV_Target {
  uint acc = 0;

// CHECK:       [[input_x_ref:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %inputs %int_0
// CHECK-NEXT:  [[input_x_val:%[0-9]+]] = OpLoad %uint [[input_x_ref]]
// CHECK-NEXT:  [[input_y_ref:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %inputs %int_1
// CHECK-NEXT:  [[input_y_val:%[0-9]+]] = OpLoad %uint [[input_y_ref]]
// CHECK-NEXT:  [[a0:%[0-9]+]] = OpLoad %uint %acc0
// CHECK-NEXT:  [[t0:%[0-9]+]] = OpUDot %uint [[input_x_val]] [[input_y_val]] PackedVectorFormat4x8Bit
// CHECK-NEXT:  [[t1:%[0-9]+]] = OpIAdd %uint [[t0]] [[a0]]
  acc += dot4add_u8packed(inputs.x, inputs.y, acc0);

// CHECK:       [[input_z_ref:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %inputs %int_2
// CHECK-NEXT:  [[input_z_val:%[0-9]+]] = OpLoad %uint [[input_z_ref]]
// CHECK-NEXT:  [[input_w_ref:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %inputs %int_3
// CHECK-NEXT:  [[input_w_val:%[0-9]+]] = OpLoad %uint [[input_w_ref]]
// CHECK-NEXT:  [[a1:%[0-9]+]] = OpLoad %int %acc1
// CHECK-NEXT:  [[t2:%[0-9]+]] = OpSDot %int [[input_z_val]] [[input_w_val]] PackedVectorFormat4x8Bit
// CHECK-NEXT:  [[t3:%[0-9]+]] = OpIAdd %int [[t2]] [[a1]]
  acc += dot4add_i8packed(inputs.z, inputs.w, acc1);

  return acc;
}
