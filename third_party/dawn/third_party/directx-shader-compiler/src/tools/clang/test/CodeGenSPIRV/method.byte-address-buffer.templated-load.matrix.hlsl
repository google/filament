// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

void foo(float16_t2x3 param[3]) {}

ByteAddressBuffer buf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId)
{
// ********* 16-bit matrix ********************

// CHECK:         [[index:%[0-9]+]] = OpShiftRightLogical %uint [[addr0:%[0-9]+]] %uint_2
// CHECK:       [[byteOff:%[0-9]+]] = OpUMod %uint [[addr0]] %uint_4
// CHECK:        [[bitOff:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff]] %uint_3
// CHECK:           [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index]]
// CHECK:         [[word0:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[shifted_word0:%[0-9]+]] = OpShiftRightLogical %uint [[word0]] [[bitOff]]
// CHECK:          [[val0:%[0-9]+]] = OpUConvert %ushort [[shifted_word0]]
// CHECK:         [[addr1:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_2
// CHECK:         [[index_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr1]] %uint_2
// CHECK:       [[byteOff_0:%[0-9]+]] = OpUMod %uint [[addr1]] %uint_4
// CHECK:        [[bitOff_0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_0]] %uint_3
// CHECK:           [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
// CHECK:         [[word0_0:%[0-9]+]] = OpLoad %uint [[ptr_0]]
// CHECK: [[shifted_word0_0:%[0-9]+]] = OpShiftRightLogical %uint [[word0_0]] [[bitOff_0]]
// CHECK:          [[val1:%[0-9]+]] = OpUConvert %ushort [[shifted_word0_0]]
// CHECK:         [[addr2:%[0-9]+]] = OpIAdd %uint [[addr1]] %uint_2
// CHECK:       [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr2]] %uint_2
// CHECK:       [[byteOff_1:%[0-9]+]] = OpUMod %uint [[addr2]] %uint_4
// CHECK:        [[bitOff_1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_1]] %uint_3
// CHECK:           [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:         [[word1:%[0-9]+]] = OpLoad %uint [[ptr_1]]
// CHECK: [[shifted_word1:%[0-9]+]] = OpShiftRightLogical %uint [[word1]] [[bitOff_1]]
// CHECK:          [[val2:%[0-9]+]] = OpUConvert %ushort [[shifted_word1]]
// CHECK:         [[addr3:%[0-9]+]] = OpIAdd %uint [[addr2]] %uint_2
// CHECK:       [[index_1_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr3]] %uint_2
// CHECK:       [[byteOff_2:%[0-9]+]] = OpUMod %uint [[addr3]] %uint_4
// CHECK:        [[bitOff_2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_2]] %uint_3
// CHECK:           [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_0]]
// CHECK:         [[word1_0:%[0-9]+]] = OpLoad %uint [[ptr_2]]
// CHECK: [[shifted_word1_0:%[0-9]+]] = OpShiftRightLogical %uint [[word1_0]] [[bitOff_2]]
// CHECK:          [[val3:%[0-9]+]] = OpUConvert %ushort [[shifted_word1_1:%[0-9]+]]
// CHECK:         [[addr4:%[0-9]+]] = OpIAdd %uint [[addr3]] %uint_2
// CHECK:       [[index_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr4]] %uint_2
// CHECK:       [[byteOff_3:%[0-9]+]] = OpUMod %uint [[addr4]] %uint_4
// CHECK:        [[bitOff_3:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_3]] %uint_3
// CHECK:           [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:         [[word2:%[0-9]+]] = OpLoad %uint [[ptr_3]]
// CHECK: [[shifted_word2:%[0-9]+]] = OpShiftRightLogical %uint [[word2]] [[bitOff_3]]
// CHECK:          [[val4:%[0-9]+]] = OpUConvert %ushort [[shifted_word2_0:%[0-9]+]]
// CHECK:         [[addr5:%[0-9]+]] = OpIAdd %uint [[addr4]] %uint_2
// CHECK:       [[index_2_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr5]] %uint_2
// CHECK:       [[byteOff_4:%[0-9]+]] = OpUMod %uint [[addr5]] %uint_4
// CHECK:        [[bitOff_4:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_4]] %uint_3
// CHECK:           [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_0]]
// CHECK:         [[word2_0:%[0-9]+]] = OpLoad %uint [[ptr_4]]
// CHECK: [[shifted_word2_1:%[0-9]+]] = OpShiftRightLogical %uint [[word2_0]] [[bitOff_4]]
// CHECK:          [[val5:%[0-9]+]] = OpUConvert %ushort [[shifted_word2_2:%[0-9]+]]
// CHECK:          [[row0:%[0-9]+]] = OpCompositeConstruct %v3ushort [[val0]] [[val2]] [[val4]]
// CHECK:          [[row1:%[0-9]+]] = OpCompositeConstruct %v3ushort [[val1]] [[val3]] [[val5]]
// CHECK:        [[matrix:%[0-9]+]] = OpCompositeConstruct %_arr_v3ushort_uint_2 [[row0]] [[row1]]
// CHECK:                          OpStore %u16 [[matrix]]
  uint16_t2x3 u16 = buf.Load<uint16_t2x3>(tid.x);

// ********* 32-bit matrix ********************

// CHECK:[[index_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_0:%[0-9]+]] %uint_2
// CHECK:    [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
// CHECK:  [[word0_1:%[0-9]+]] = OpLoad %uint [[ptr_5]]
// CHECK:   [[val0_0:%[0-9]+]] = OpBitcast %int [[word0_1]]
// CHECK:[[index_1_1:%[0-9]+]] = OpIAdd %uint [[index_0]] %uint_1
// CHECK:    [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_1]]
// CHECK:  [[word1_1:%[0-9]+]] = OpLoad %uint [[ptr_6]]
// CHECK:   [[val1_0:%[0-9]+]] = OpBitcast %int [[word1_2:%[0-9]+]]
// CHECK:[[index_2_1:%[0-9]+]] = OpIAdd %uint [[index_1_1]] %uint_1
// CHECK:    [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_1]]
// CHECK:  [[word2_1:%[0-9]+]] = OpLoad %uint [[ptr_7]]
// CHECK:   [[val2_0:%[0-9]+]] = OpBitcast %int [[word2_1]]
// CHECK:[[index_3:%[0-9]+]] = OpIAdd %uint [[index_2_1]] %uint_1
// CHECK:    [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:  [[word3:%[0-9]+]] = OpLoad %uint [[ptr_8]]
// CHECK:   [[val3_0:%[0-9]+]] = OpBitcast %int [[word3]]
// CHECK:[[index_4:%[0-9]+]] = OpIAdd %uint [[index_3]] %uint_1
// CHECK:    [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4]]
// CHECK:  [[word4:%[0-9]+]] = OpLoad %uint [[ptr_9]]
// CHECK:   [[val4_0:%[0-9]+]] = OpBitcast %int [[word4]]
// CHECK:[[index_5:%[0-9]+]] = OpIAdd %uint [[index_4]] %uint_1
// CHECK:    [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5]]
// CHECK:  [[word5:%[0-9]+]] = OpLoad %uint [[ptr_10]]
// CHECK:   [[val5_0:%[0-9]+]] = OpBitcast %int [[word5]]
// CHECK:   [[row0_0:%[0-9]+]] = OpCompositeConstruct %v2int [[val0_0]] [[val3_0]]
// CHECK:   [[row1_0:%[0-9]+]] = OpCompositeConstruct %v2int [[val1_0]] [[val4_0]]
// CHECK:   [[row2:%[0-9]+]] = OpCompositeConstruct %v2int [[val2_0]] [[val5_0]]
// CHECK: [[matrix_0:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[row0_0]] [[row1_0]] [[row2]]
// CHECK:                   OpStore %j [[matrix_0]]
  int3x2 j = buf.Load<int3x2>(tid.x);

// ********* 64-bit matrix ********************

// CHECK:             [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_1:%[0-9]+]] %uint_2
// CHECK:              [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:             [[word0_2:%[0-9]+]] = OpLoad %uint [[ptr_11]]
// CHECK:           [[index_1_2:%[0-9]+]] = OpIAdd %uint [[index_1]] %uint_1
// CHECK:              [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_2]]
// CHECK:             [[word1_3:%[0-9]+]] = OpLoad %uint [[ptr_12]]
// CHECK:           [[index_2_2:%[0-9]+]] = OpIAdd %uint [[index_1_2]] %uint_1
// CHECK:               [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0_2]] [[word1_3]]
// CHECK:              [[val0_1:%[0-9]+]] = OpBitcast %double [[merge]]

// CHECK:              [[ptr_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_2]]
// CHECK:             [[word2_2:%[0-9]+]] = OpLoad %uint [[ptr_13]]
// CHECK:           [[index_3_0:%[0-9]+]] = OpIAdd %uint [[index_2_2]] %uint_1
// CHECK:              [[ptr_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3_0]]
// CHECK:             [[word3_0:%[0-9]+]] = OpLoad %uint [[ptr_14]]
// CHECK:           [[index_4_0:%[0-9]+]] = OpIAdd %uint [[index_3_0]] %uint_1
// CHECK:               [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word2_2]] [[word3_0]]
// CHECK:              [[val1_1:%[0-9]+]] = OpBitcast %double [[merge]]

// CHECK:              [[ptr_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4_0]]
// CHECK:             [[word4_0:%[0-9]+]] = OpLoad %uint [[ptr_15]]
// CHECK:           [[index_5_0:%[0-9]+]] = OpIAdd %uint [[index_4_0]] %uint_1
// CHECK:              [[ptr_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5_0]]
// CHECK:             [[word5_0:%[0-9]+]] = OpLoad %uint [[ptr_16]]
// CHECK:             [[index_6:%[0-9]+]] = OpIAdd %uint [[index_5_0]] %uint_1
// CHECK:               [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word4_0]] [[word5_0]]
// CHECK:              [[val2_1:%[0-9]+]] = OpBitcast %double [[merge]]

// CHECK:              [[ptr_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_6]]
// CHECK:               [[word6:%[0-9]+]] = OpLoad %uint [[ptr_17]]
// CHECK:             [[index_7:%[0-9]+]] = OpIAdd %uint [[index_6]] %uint_1
// CHECK:              [[ptr_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_7]]
// CHECK:               [[word7:%[0-9]+]] = OpLoad %uint [[ptr_18]]
// CHECK:             [[index_8:%[0-9]+]] = OpIAdd %uint [[index_7]] %uint_1
// CHECK:               [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word6]] [[word7]]
// CHECK:              [[val3_1:%[0-9]+]] = OpBitcast %double [[merge]]

// CHECK:              [[row0_1:%[0-9]+]] = OpCompositeConstruct %v2double [[val0_1]] [[val2_1]]
// CHECK:              [[row1_1:%[0-9]+]] = OpCompositeConstruct %v2double [[val1_1]] [[val3_1]]
// CHECK:            [[matrix_1:%[0-9]+]] = OpCompositeConstruct %mat2v2double [[row0_1]] [[row1_1]]
// CHECK:                                   OpStore %f64 [[matrix_1]]
  float64_t2x2 f64 = buf.Load<float64_t2x2>(tid.x);

// ********* array of matrices ********************

// CHECK:             [[index_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_2:%[0-9]+]] %uint_2
// CHECK:                 [[ptr_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:               [[addr1_0:%[0-9]+]] = OpIAdd %uint [[addr0_2]] %uint_2
// CHECK:             [[index_3:%[0-9]+]] = OpShiftRightLogical %uint [[addr1_0]] %uint_2
// CHECK:                 [[ptr_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:               [[addr2_0:%[0-9]+]] = OpIAdd %uint [[addr1_0]] %uint_2
// CHECK:             [[index_1_3:%[0-9]+]] = OpShiftRightLogical %uint [[addr2_0]] %uint_2
// CHECK:                 [[ptr_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_3]]
// CHECK:               [[addr3_0:%[0-9]+]] = OpIAdd %uint [[addr2_0]] %uint_2
// CHECK:             [[index_1_4:%[0-9]+]] = OpShiftRightLogical %uint [[addr3_0]] %uint_2
// CHECK:                 [[ptr_22:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_4]]
// CHECK:               [[addr4_0:%[0-9]+]] = OpIAdd %uint [[addr3_0]] %uint_2
// CHECK:             [[index_2_3:%[0-9]+]] = OpShiftRightLogical %uint [[addr4_0]] %uint_2
// CHECK:                 [[ptr_23:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_3]]
// CHECK:               [[addr5_0:%[0-9]+]] = OpIAdd %uint [[addr4_0]] %uint_2
// CHECK:             [[index_2_4:%[0-9]+]] = OpShiftRightLogical %uint [[addr5_0]] %uint_2
// CHECK:                 [[ptr_24:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_4]]
// CHECK:               [[addr6:%[0-9]+]] = OpIAdd %uint [[addr5_0]] %uint_2
// CHECK:                [[row1_2:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:                [[row2_0:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:            [[matrix_1:%[0-9]+]] = OpCompositeConstruct %mat2v3half [[row1_2]] [[row2_0]]
// CHECK:             [[index_3_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr6]] %uint_2
// CHECK:                 [[ptr_25:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3_1]]
// CHECK:               [[addr7:%[0-9]+]] = OpIAdd %uint [[addr6]] %uint_2
// CHECK:             [[index_3_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr7]] %uint_2
// CHECK:                 [[ptr_26:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3_2]]
// CHECK:               [[addr8:%[0-9]+]] = OpIAdd %uint [[addr7]] %uint_2
// CHECK:             [[index_4_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr8]] %uint_2
// CHECK:                 [[ptr_27:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4_1]]
// CHECK:               [[addr9:%[0-9]+]] = OpIAdd %uint [[addr8]] %uint_2
// CHECK:             [[index_4_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr9]] %uint_2
// CHECK:                 [[ptr_28:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4_2]]
// CHECK:              [[addr10:%[0-9]+]] = OpIAdd %uint [[addr9]] %uint_2
// CHECK:             [[index_5_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr10]] %uint_2
// CHECK:                 [[ptr_29:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5_1]]
// CHECK:              [[addr11:%[0-9]+]] = OpIAdd %uint [[addr10]] %uint_2
// CHECK:             [[index_5_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr11]] %uint_2
// CHECK:                 [[ptr_30:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5_2]]
// CHECK:              [[addr12:%[0-9]+]] = OpIAdd %uint [[addr11]] %uint_2
// CHECK:                [[row1_3:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:                [[row2_1:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:            [[matrix_2:%[0-9]+]] = OpCompositeConstruct %mat2v3half [[row1_3]] [[row2_1]]
// CHECK:             [[index_6_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr12]] %uint_2
// CHECK:                 [[ptr_31:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_6_0]]
// CHECK:              [[addr13:%[0-9]+]] = OpIAdd %uint [[addr12]] %uint_2
// CHECK:             [[index_6_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr13]] %uint_2
// CHECK:                 [[ptr_32:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_6_1]]
// CHECK:              [[addr14:%[0-9]+]] = OpIAdd %uint [[addr13]] %uint_2
// CHECK:             [[index_7_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr14]] %uint_2
// CHECK:                 [[ptr_33:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_7_0]]
// CHECK:              [[addr15:%[0-9]+]] = OpIAdd %uint [[addr14]] %uint_2
// CHECK:             [[index_7_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr15]] %uint_2
// CHECK:                 [[ptr_34:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_7_1]]
// CHECK:              [[addr16:%[0-9]+]] = OpIAdd %uint [[addr15]] %uint_2
// CHECK:             [[index_8:%[0-9]+]] = OpShiftRightLogical %uint [[addr16]] %uint_2
// CHECK:                 [[ptr_35:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_8]]
// CHECK:              [[addr17:%[0-9]+]] = OpIAdd %uint [[addr16]] %uint_2
// CHECK:             [[index_8_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr17]] %uint_2
// CHECK:                 [[ptr_36:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_8_0]]
// CHECK:                [[row1_4:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:                [[row2_2:%[0-9]+]] = OpCompositeConstruct %v3half
// CHECK:            [[matrix_3:%[0-9]+]] = OpCompositeConstruct %mat2v3half [[row1_4]] [[row2_2]]
// CHECK:        [[matrix_array:%[0-9]+]] = OpCompositeConstruct %_arr_mat2v3half_uint_3 [[matrix_1]] [[matrix_2]] [[matrix_3]]
// CHECK:                                OpStore %matVec [[matrix_array]]
  float16_t2x3 matVec[3] = buf.Load<float16_t2x3[3]>(tid.x);

//
// Check that the rvalue resulting from the templated load is accessed correctly
// A temporary LValue has to be constructed and accessed in order to do this.
//
// CHECK: OpCompositeConstruct %_arr_mat2v3half_uint_3
// CHECK: OpStore %temp_var_
// CHECK: OpAccessChain %_ptr_Function_mat2v3half %temp_var_ %int_0
// CHECK: OpLoad %mat2v3half
// CHECK: OpCompositeExtract %half {{%[0-9]+}} 0 1
// CHECK: OpCompositeExtract %half {{%[0-9]+}} 0 2
// CHECK: OpCompositeConstruct %v2half
// CHECK: OpStore %customMatrix {{%[0-9]+}}
  float16_t2 customMatrix = (buf.Load<float16_t2x3[3]>(tid.x))[0]._m01_m02;

// CHECK: OpCompositeConstruct %_arr_mat2v3half_uint_3
// CHECK: OpStore %temp_var_vector
// CHECK: OpAccessChain %_ptr_Function_half %temp_var_vector %int_1 %uint_0 %uint_1
// CHECK: OpLoad %half
// CHECK: OpCompositeConstruct %_arr_half_uint_3
// CHECK: OpStore %a {{%[0-9]+}}
  half a[3] = {1, (buf.Load<float16_t2x3[3]>(tid.x))[1][0][1], 0};

// CHECK: OpCompositeConstruct %_arr_mat2v3half_uint_3
// CHECK: OpStore %param_var_param {{%[0-9]+}}
// CHECK: OpFunctionCall %void %foo %param_var_param
  foo(buf.Load<float16_t2x3[3]>(tid.x));
}
