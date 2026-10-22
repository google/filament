// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer buf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId)
{
// ********* 16-bit vector ********************
// CHECK:   [[index_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0:%[0-9]+]] %uint_2
// CHECK:  [[byteOff0:%[0-9]+]] = OpUMod %uint [[addr0]] %uint_4
// CHECK:   [[bitOff0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff0]] %uint_3
// CHECK:       [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
// CHECK:     [[word0:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[val0_uint:%[0-9]+]] = OpShiftRightLogical %uint [[word0]] [[bitOff0]]
// CHECK:      [[val0:%[0-9]+]] = OpUConvert %ushort [[val0_uint]]
// CHECK:     [[addr1:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_2

// CHECK:   [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr1]] %uint_2
// CHECK:  [[byteOff1:%[0-9]+]] = OpUMod %uint [[addr1]] %uint_4
// CHECK:   [[bitOff1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff1]] %uint_3
// CHECK:       [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:     [[word0_0:%[0-9]+]] = OpLoad %uint [[ptr_0]]
// CHECK: [[val1_uint:%[0-9]+]] = OpShiftRightLogical %uint [[word0_0]] [[bitOff1]]
// CHECK:      [[val1:%[0-9]+]] = OpUConvert %ushort [[val1_uint]]
// CHECK:     [[addr2:%[0-9]+]] = OpIAdd %uint [[addr1]] %uint_2

// CHECK:   [[index_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr2_0:%[0-9]+]] %uint_2
// CHECK:  [[byteOff2:%[0-9]+]] = OpUMod %uint [[addr2_0]] %uint_4
// CHECK:   [[bitOff2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff2]] %uint_3
// CHECK:       [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:     [[word1:%[0-9]+]] = OpLoad %uint [[ptr_1]]
// CHECK: [[val2_uint:%[0-9]+]] = OpShiftRightLogical %uint [[word1]] [[bitOff2]]
// CHECK:      [[val2:%[0-9]+]] = OpUConvert %ushort [[val2_uint]]

// CHECK:      [[uVec:%[0-9]+]] = OpCompositeConstruct %v3ushort [[val0]] [[val1]] [[val2]]
// CHECK:                      OpStore %u16 [[uVec]]
  uint16_t3 u16 = buf.Load<uint16_t3>(tid.x);


// ********* 32-bit vector ********************

// CHECK: [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_0:%[0-9]+]] %uint_2
// CHECK:     [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:   [[word0_1:%[0-9]+]] = OpLoad %uint [[ptr_2]]
// CHECK:    [[val0_0:%[0-9]+]] = OpBitcast %int [[word0_1]]
// CHECK: [[index_1_0:%[0-9]+]] = OpIAdd %uint [[index_1]] %uint_1
// CHECK:     [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_0]]
// CHECK:   [[word1_0:%[0-9]+]] = OpLoad %uint [[ptr_3]]
// CHECK:    [[val1_0:%[0-9]+]] = OpBitcast %int [[word1_0]]
// CHECK:    [[iVec:%[0-9]+]] = OpCompositeConstruct %v2int [[val0_0]] [[val1_0]]
// CHECK:                    OpStore %i [[iVec]]
  int2 i = buf.Load<int2>(tid.x);

// CHECK: [[index_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_1:%[0-9]+]] %uint_2
// CHECK:     [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:   [[word0_2:%[0-9]+]] = OpLoad %uint [[ptr_4]]
// CHECK:    [[val0_1:%[0-9]+]] = OpINotEqual %bool [[word0_2]] %uint_0
// CHECK: [[index_1_1:%[0-9]+]] = OpIAdd %uint [[index_2]] %uint_1
// CHECK:     [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_1]]
// CHECK:   [[word1_1:%[0-9]+]] = OpLoad %uint [[ptr_5]]
// CHECK:    [[val1_1:%[0-9]+]] = OpINotEqual %bool [[word1_1]] %uint_0
// CHECK:    [[bVec:%[0-9]+]] = OpCompositeConstruct %v2bool [[val0_1]] [[val1_1]]
// CHECK:                    OpStore %b [[bVec]]
  bool2 b = buf.Load<bool2>(tid.x);

// ********* 64-bit vector ********************

// CHECK:   [[index_3:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_2:%[0-9]+]] %uint_2
// CHECK:     [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:   [[word0_3:%[0-9]+]] = OpLoad %uint [[ptr_6]]
// CHECK: [[index_1_2:%[0-9]+]] = OpIAdd %uint [[index_3]] %uint_1
// CHECK:     [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_2]]
// CHECK:   [[word1_2:%[0-9]+]] = OpLoad %uint [[ptr_7]]
// CHECK: [[index_2_0:%[0-9]+]] = OpIAdd %uint [[index_1_2]] %uint_1
// CHECK:     [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0_3]] [[word1_2]]
// CHECK:    [[val0_2:%[0-9]+]] = OpBitcast %double [[merge]]
// CHECK:     [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_0]]
// CHECK:   [[word0_4:%[0-9]+]] = OpLoad %uint [[ptr_8]]
// CHECK:   [[index_3:%[0-9]+]] = OpIAdd %uint [[index_2_0]] %uint_1
// CHECK:     [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:   [[word1_3:%[0-9]+]] = OpLoad %uint [[ptr_9]]
// CHECK:   [[index_4:%[0-9]+]] = OpIAdd %uint [[index_3]] %uint_1
// CHECK:     [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0_4]] [[word1_3]]
// CHECK:    [[val1_2:%[0-9]+]] = OpBitcast %double [[merge]]
// CHECK:      [[fVec:%[0-9]+]] = OpCompositeConstruct %v2double [[val0_2]] [[val1_2]]
// CHECK:                         OpStore %f64 [[fVec]]
  float64_t2 f64 = buf.Load<float64_t2>(tid.x);

// ********* array of vectors ********************

// CHECK:       [[index_4:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_3:%[0-9]+]] %uint_2
// CHECK:       [[byteOff:%[0-9]+]] = OpUMod %uint [[addr0_3]] %uint_4
// CHECK:        [[bitOff:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff]] %uint_3
// CHECK:           [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4]]
// CHECK:         [[word0_5:%[0-9]+]] = OpLoad %uint [[ptr_10]]
// CHECK:     [[val0_uint_0:%[0-9]+]] = OpShiftRightLogical %uint [[word0_5]] [[bitOff]]
// CHECK:          [[val0_3:%[0-9]+]] = OpUConvert %ushort [[val0_uint_0]]
// CHECK:         [[addr1_0:%[0-9]+]] = OpIAdd %uint [[addr0_3]] %uint_2

// CHECK:       [[index_5:%[0-9]+]] = OpShiftRightLogical %uint [[addr1_0]] %uint_2
// CHECK:       [[byteOff_0:%[0-9]+]] = OpUMod %uint [[addr1_0]] %uint_4
// CHECK:        [[bitOff_0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_0]] %uint_3
// CHECK:           [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5]]
// CHECK:         [[word0_6:%[0-9]+]] = OpLoad %uint [[ptr_11]]
// CHECK: [[word0_shifted:%[0-9]+]] = OpShiftRightLogical %uint [[word0_6]] [[bitOff_0]]
// CHECK:          [[val1_3:%[0-9]+]] = OpUConvert %ushort [[word0_shifted]]
// CHECK:         [[addr2_1:%[0-9]+]] = OpIAdd %uint [[addr1_0]] %uint_2

// CHECK:       [[index_1_3:%[0-9]+]] = OpShiftRightLogical %uint [[addr2_1]] %uint_2
// CHECK:       [[byteOff_1:%[0-9]+]] = OpUMod %uint [[addr2_1]] %uint_4
// CHECK:        [[bitOff_1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_1]] %uint_3
// CHECK:           [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_3]]
// CHECK:         [[word1_4:%[0-9]+]] = OpLoad %uint [[ptr_12]]
// CHECK: [[word1_shifted:%[0-9]+]] = OpShiftRightLogical %uint [[word1_4]] [[bitOff_1]]
// CHECK:          [[val2_0:%[0-9]+]] = OpUConvert %ushort [[word1_shifted]]
// CHECK:         [[addr3:%[0-9]+]] = OpIAdd %uint [[addr2_1]] %uint_2

// CHECK:          [[vec0:%[0-9]+]] = OpCompositeConstruct %v3ushort [[val0_3]] [[val1_3]] [[val2_0]]

// CHECK:       [[index_1_4:%[0-9]+]] = OpShiftRightLogical %uint [[addr3]] %uint_2
// CHECK:       [[byteOff_2:%[0-9]+]] = OpUMod %uint [[addr3]] %uint_4
// CHECK:        [[bitOff_2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_2]] %uint_3
// CHECK:           [[ptr_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1_4]]
// CHECK:         [[word1_5:%[0-9]+]] = OpLoad %uint [[ptr_13]]
// CHECK: [[word1_shifted_0:%[0-9]+]] = OpShiftRightLogical %uint [[word1_5]] [[bitOff_2]]
// CHECK:          [[val3:%[0-9]+]] = OpUConvert %ushort [[word1_shifted_1:%[0-9]+]]
// CHECK:         [[addr4:%[0-9]+]] = OpIAdd %uint [[addr3]] %uint_2

// CHECK:       [[index_2_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr4]] %uint_2
// CHECK:       [[byteOff_3:%[0-9]+]] = OpUMod %uint [[addr4]] %uint_4
// CHECK:        [[bitOff_3:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_3]] %uint_3
// CHECK:           [[ptr_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_1]]
// CHECK:         [[word2:%[0-9]+]] = OpLoad %uint [[ptr_14]]
// CHECK: [[word2_shifted:%[0-9]+]] = OpShiftRightLogical %uint [[word2]] [[bitOff_3]]
// CHECK:          [[val4:%[0-9]+]] = OpUConvert %ushort [[word2_shifted]]
// CHECK:         [[addr5:%[0-9]+]] = OpIAdd %uint [[addr4]] %uint_2

// CHECK:       [[index_2_2:%[0-9]+]] = OpShiftRightLogical %uint [[addr5]] %uint_2
// CHECK:       [[byteOff_4:%[0-9]+]] = OpUMod %uint [[addr5]] %uint_4
// CHECK:        [[bitOff_4:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_4]] %uint_3
// CHECK:           [[ptr_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2_2]]
// CHECK:         [[word2_0:%[0-9]+]] = OpLoad %uint [[ptr_15]]
// CHECK: [[shifted_word2:%[0-9]+]] = OpShiftRightLogical %uint [[word2_0]] [[bitOff_4]]
// CHECK:          [[val5:%[0-9]+]] = OpUConvert %ushort [[shifted_word2]]
// CHECK:         [[addr6:%[0-9]+]] = OpIAdd %uint [[addr5]] %uint_2

// CHECK:          [[vec1:%[0-9]+]] = OpCompositeConstruct %v3ushort [[val3]] [[val4]] [[val5]]

// CHECK:       [[index_3_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr6]] %uint_2
// CHECK:       [[byteOff_5:%[0-9]+]] = OpUMod %uint [[addr6]] %uint_4
// CHECK:        [[bitOff_5:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_5]] %uint_3
// CHECK:           [[ptr_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3_0]]
// CHECK:         [[word3:%[0-9]+]] = OpLoad %uint [[ptr_16]]
// CHECK: [[word3_shifted:%[0-9]+]] = OpShiftRightLogical %uint [[word3]] [[bitOff_5]]
// CHECK:          [[val6:%[0-9]+]] = OpUConvert %ushort [[word3_shifted]]
// CHECK:         [[addr7:%[0-9]+]] = OpIAdd %uint [[addr6]] %uint_2

// CHECK:       [[index_3_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr7]] %uint_2
// CHECK:       [[byteOff_6:%[0-9]+]] = OpUMod %uint [[addr7]] %uint_4
// CHECK:        [[bitOff_6:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_6]] %uint_3
// CHECK:           [[ptr_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3_1]]
// CHECK:         [[word3_0:%[0-9]+]] = OpLoad %uint [[ptr_17]]
// CHECK: [[shifted_word3:%[0-9]+]] = OpShiftRightLogical %uint [[word3_0]] [[bitOff_6]]
// CHECK:          [[val7:%[0-9]+]] = OpUConvert %ushort [[shifted_word3]]
// CHECK:         [[addr8:%[0-9]+]] = OpIAdd %uint [[addr7]] %uint_2

// CHECK:       [[index_4:%[0-9]+]] = OpShiftRightLogical %uint [[addr8]] %uint_2
// CHECK:       [[byteOff_7:%[0-9]+]] = OpUMod %uint [[addr8]] %uint_4
// CHECK:        [[bitOff_7:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_7]] %uint_3
// CHECK:           [[ptr_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4]]
// CHECK:         [[word4:%[0-9]+]] = OpLoad %uint [[ptr_18]]
// CHECK: [[word4_shifted:%[0-9]+]] = OpShiftRightLogical %uint [[word4]] [[bitOff_7]]
// CHECK:          [[val8:%[0-9]+]] = OpUConvert %ushort [[word4_shifted]]

// CHECK:          [[vec2:%[0-9]+]] = OpCompositeConstruct %v3ushort [[val6]] [[val7]] [[val8]]
// CHECK:        [[vecArr:%[0-9]+]] = OpCompositeConstruct %_arr_v3ushort_uint_3 [[vec0]] [[vec1]] [[vec2]]
// CHECK:                          OpStore %uVec [[vecArr]]
  uint16_t3 uVec[3] = buf.Load<uint16_t3[3]>(tid.x);
}

