// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-DAG: [[type_2d_image_array_ms:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 1 1 Unknown
// CHECK-DAG: [[type_2d_sampled_image_array_ms:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array_ms]]

// CHECK-DAG:   [[pos2_0:%[0-9]+]] = OpConstantComposite %v2float %float_0_25 %float_0_25
// CHECK-DAG:   [[pos2_1:%[0-9]+]] = OpConstantComposite %v2float %float_n0_25 %float_n0_25
// CHECK-DAG:     [[pos2:%[0-9]+]] = OpConstantComposite %_arr_v2float_uint_2 [[pos2_0]] [[pos2_1]]

// CHECK-DAG:   [[pos4_0:%[0-9]+]] = OpConstantComposite %v2float %float_n0_125 %float_n0_375
// CHECK-DAG:   [[pos4_1:%[0-9]+]] = OpConstantComposite %v2float %float_0_375 %float_n0_125
// CHECK-DAG:   [[pos4_2:%[0-9]+]] = OpConstantComposite %v2float %float_n0_375 %float_0_125
// CHECK-DAG:   [[pos4_3:%[0-9]+]] = OpConstantComposite %v2float %float_0_125 %float_0_375
// CHECK-DAG:     [[pos4:%[0-9]+]] = OpConstantComposite %_arr_v2float_uint_4 [[pos4_0]] [[pos4_1]] [[pos4_2]] [[pos4_3]]

// CHECK-DAG:   [[pos8_0:%[0-9]+]] = OpConstantComposite %v2float %float_0_0625 %float_n0_1875
// CHECK-DAG:   [[pos8_1:%[0-9]+]] = OpConstantComposite %v2float %float_n0_0625 %float_0_1875
// CHECK-DAG:   [[pos8_2:%[0-9]+]] = OpConstantComposite %v2float %float_0_3125 %float_0_0625
// CHECK-DAG:   [[pos8_3:%[0-9]+]] = OpConstantComposite %v2float %float_n0_1875 %float_n0_3125
// CHECK-DAG:   [[pos8_4:%[0-9]+]] = OpConstantComposite %v2float %float_n0_3125 %float_0_3125
// CHECK-DAG:   [[pos8_5:%[0-9]+]] = OpConstantComposite %v2float %float_n0_4375 %float_n0_0625
// CHECK-DAG:   [[pos8_6:%[0-9]+]] = OpConstantComposite %v2float %float_0_1875 %float_0_4375
// CHECK-DAG:   [[pos8_7:%[0-9]+]] = OpConstantComposite %v2float %float_0_4375 %float_n0_4375
// CHECK-DAG:     [[pos8:%[0-9]+]] = OpConstantComposite %_arr_v2float_uint_8 [[pos8_0]] [[pos8_1]] [[pos8_2]] [[pos8_3]] [[pos8_4]] [[pos8_5]] [[pos8_6]] [[pos8_7]]

// CHECK-DAG: [[pos16_00:%[0-9]+]] = OpConstantComposite %v2float %float_0_0625 %float_0_0625
// CHECK-DAG: [[pos16_01:%[0-9]+]] = OpConstantComposite %v2float %float_n0_0625 %float_n0_1875
// CHECK-DAG: [[pos16_02:%[0-9]+]] = OpConstantComposite %v2float %float_n0_1875 %float_0_125
// CHECK-DAG: [[pos16_03:%[0-9]+]] = OpConstantComposite %v2float %float_0_25 %float_n0_0625
// CHECK-DAG: [[pos16_04:%[0-9]+]] = OpConstantComposite %v2float %float_n0_3125 %float_n0_125
// CHECK-DAG: [[pos16_05:%[0-9]+]] = OpConstantComposite %v2float %float_0_125 %float_0_3125
// CHECK-DAG: [[pos16_06:%[0-9]+]] = OpConstantComposite %v2float %float_0_3125 %float_0_1875
// CHECK-DAG: [[pos16_07:%[0-9]+]] = OpConstantComposite %v2float %float_0_1875 %float_n0_3125
// CHECK-DAG: [[pos16_08:%[0-9]+]] = OpConstantComposite %v2float %float_n0_125 %float_0_375
// CHECK-DAG: [[pos16_09:%[0-9]+]] = OpConstantComposite %v2float %float_0 %float_n0_4375
// CHECK-DAG: [[pos16_10:%[0-9]+]] = OpConstantComposite %v2float %float_n0_25 %float_n0_375
// CHECK-DAG: [[pos16_11:%[0-9]+]] = OpConstantComposite %v2float %float_n0_375 %float_0_25
// CHECK-DAG: [[pos16_12:%[0-9]+]] = OpConstantComposite %v2float %float_n0_5 %float_0
// CHECK-DAG: [[pos16_13:%[0-9]+]] = OpConstantComposite %v2float %float_0_4375 %float_n0_25
// CHECK-DAG: [[pos16_14:%[0-9]+]] = OpConstantComposite %v2float %float_0_375 %float_0_4375
// CHECK-DAG: [[pos16_15:%[0-9]+]] = OpConstantComposite %v2float %float_n0_4375 %float_n0_5
// CHECK-DAG:    [[pos16:%[0-9]+]] = OpConstantComposite %_arr_v2float_uint_16 [[pos16_00]] [[pos16_01]] [[pos16_02]] [[pos16_03]] [[pos16_04]] [[pos16_05]] [[pos16_06]] [[pos16_07]] [[pos16_08]] [[pos16_09]] [[pos16_10]] [[pos16_11]] [[pos16_12]] [[pos16_13]] [[pos16_14]] [[pos16_15]]

// CHECK-DAG:     [[zero:%[0-9]+]] = OpConstantComposite %v2float %float_0 %float_0

vk::SampledTexture2DMS<float4> tex2dMS;

void main(int index : INDEX) {
// CHECK:  %var_GetSamplePosition_data_2 = OpVariable %_ptr_Function__arr_v2float_uint_2 Function
// CHECK:  %var_GetSamplePosition_data_4 = OpVariable %_ptr_Function__arr_v2float_uint_4 Function
// CHECK:  %var_GetSamplePosition_data_8 = OpVariable %_ptr_Function__arr_v2float_uint_8 Function
// CHECK: %var_GetSamplePosition_data_16 = OpVariable %_ptr_Function__arr_v2float_uint_16 Function
// CHECK:  %var_GetSamplePosition_result = OpVariable %_ptr_Function_v2float Function

// CHECK:      [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_array_ms]] %tex2dMS
// CHECK-NEXT: [[img_ms:%[0-9]+]] = OpImage [[type_2d_image_array_ms]] [[tex_ms]]
// CHECK-NEXT:[[count_ms:%[0-9]+]] = OpImageQuerySamples %uint [[img_ms]]
// CHECK-NEXT:   [[index:%[0-9]+]] = OpLoad %int %index
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_data_2 [[pos2]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_data_4 [[pos4]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_data_8 [[pos8]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_data_16 [[pos16]]

// CHECK-NEXT:   [[eq2:%[0-9]+]] = OpIEqual %bool [[count_ms]] %uint_2
// CHECK-NEXT:                  OpSelectionMerge %if_GetSamplePosition_merge2 None
// CHECK-NEXT:                  OpBranchConditional [[eq2]] %if_GetSamplePosition_then2 %if_GetSamplePosition_else2

// CHECK-NEXT: %if_GetSamplePosition_then2 = OpLabel
// CHECK-NEXT:    [[ac:%[0-9]+]] = OpAccessChain %_ptr_Function_v2float %var_GetSamplePosition_data_2 [[index]]
// CHECK-NEXT:   [[val:%[0-9]+]] = OpLoad %v2float [[ac]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_result [[val]]
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge2

// CHECK-NEXT: %if_GetSamplePosition_else2 = OpLabel
// CHECK-NEXT:   [[eq4:%[0-9]+]] = OpIEqual %bool [[count_ms]] %uint_4
// CHECK-NEXT:                  OpSelectionMerge %if_GetSamplePosition_merge4 None
// CHECK-NEXT:                  OpBranchConditional [[eq4]] %if_GetSamplePosition_then4 %if_GetSamplePosition_else4

// CHECK-NEXT: %if_GetSamplePosition_then4 = OpLabel
// CHECK-NEXT:    [[ac_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v2float %var_GetSamplePosition_data_4 [[index]]
// CHECK-NEXT:   [[val_0:%[0-9]+]] = OpLoad %v2float [[ac_0]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_result [[val_0]]
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge4

// CHECK-NEXT: %if_GetSamplePosition_else4 = OpLabel
// CHECK-NEXT:   [[eq8:%[0-9]+]] = OpIEqual %bool [[count_ms]] %uint_8
// CHECK-NEXT:                  OpSelectionMerge %if_GetSamplePosition_merge8 None
// CHECK-NEXT:                  OpBranchConditional [[eq8]] %if_GetSamplePosition_then8 %if_GetSamplePosition_else8

// CHECK-NEXT: %if_GetSamplePosition_then8 = OpLabel
// CHECK-NEXT:    [[ac_1:%[0-9]+]] = OpAccessChain %_ptr_Function_v2float %var_GetSamplePosition_data_8 [[index]]
// CHECK-NEXT:   [[val_1:%[0-9]+]] = OpLoad %v2float [[ac_1]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_result [[val_1]]
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge8

// CHECK-NEXT: %if_GetSamplePosition_else8 = OpLabel
// CHECK-NEXT:  [[eq16:%[0-9]+]] = OpIEqual %bool [[count_ms]] %uint_16
// CHECK-NEXT:                  OpSelectionMerge %if_GetSamplePosition_merge16 None
// CHECK-NEXT:                  OpBranchConditional [[eq16]] %if_GetSamplePosition_then16 %if_GetSamplePosition_else16

// CHECK-NEXT: %if_GetSamplePosition_then16 = OpLabel
// CHECK-NEXT:    [[ac_2:%[0-9]+]] = OpAccessChain %_ptr_Function_v2float %var_GetSamplePosition_data_16 [[index]]
// CHECK-NEXT:   [[val_2:%[0-9]+]] = OpLoad %v2float [[ac_2]]
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_result [[val_2]]
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge16

// CHECK-NEXT: %if_GetSamplePosition_else16 = OpLabel
// CHECK-NEXT:                  OpStore %var_GetSamplePosition_result [[zero]]
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge16

// CHECK-NEXT: %if_GetSamplePosition_merge16 = OpLabel
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge8
// CHECK-NEXT: %if_GetSamplePosition_merge8 = OpLabel
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge4
// CHECK-NEXT: %if_GetSamplePosition_merge4 = OpLabel
// CHECK-NEXT:                  OpBranch %if_GetSamplePosition_merge2
// CHECK-NEXT: %if_GetSamplePosition_merge2 = OpLabel
// CHECK-NEXT:   [[val_3:%[0-9]+]] = OpLoad %v2float %var_GetSamplePosition_result
// CHECK-NEXT:                  OpStore %posMS [[val_3]]
  float2 posMS = tex2dMS.GetSamplePosition(index);

}
