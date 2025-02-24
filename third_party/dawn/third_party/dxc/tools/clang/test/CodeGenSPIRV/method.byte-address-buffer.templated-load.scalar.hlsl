// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer buf;

[numthreads(64, 1, 1)] void main(uint3 tid
                                 : SV_DispatchThreadId) {
  // ******* 16-bit scalar, literal index *******

  // CHECK:  [[index:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
  // CHECK:   [[byte:%[0-9]+]] = OpUMod %uint %uint_0 %uint_4
  // CHECK:   [[bits:%[0-9]+]] = OpShiftLeftLogical %uint [[byte]] %uint_3
  // CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index]]
  // CHECK:   [[uint:%[0-9]+]] = OpLoad %uint [[ptr]]
  // CHECK:  [[shift:%[0-9]+]] = OpShiftRightLogical %uint [[uint]] [[bits]]
  // CHECK: [[ushort:%[0-9]+]] = OpUConvert %ushort [[shift]]
  // CHECK:                   OpStore %v1 [[ushort]]
  uint16_t v1 = buf.Load<uint16_t>(0);

  // CHECK:  [[index_0:%[0-9]+]] = OpShiftRightLogical %uint %uint_2 %uint_2
  // CHECK:   [[byte_0:%[0-9]+]] = OpUMod %uint %uint_2 %uint_4
  // CHECK:   [[bits_0:%[0-9]+]] = OpShiftLeftLogical %uint [[byte_0]] %uint_3
  // CHECK:    [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
  // CHECK:   [[uint_0:%[0-9]+]] = OpLoad %uint [[ptr_0]]
  // CHECK:  [[shift_0:%[0-9]+]] = OpShiftRightLogical %uint [[uint_0]] [[bits_0]]
  // CHECK: [[ushort_0:%[0-9]+]] = OpUConvert %ushort [[shift_0]]
  // CHECK:                   OpStore %v2 [[ushort_0]]
  uint16_t v2 = buf.Load<uint16_t>(2);

  // ********* 16-bit scalar ********************

  // CHECK:   [[byte_1:%[0-9]+]] = OpUMod %uint {{%[0-9]+}} %uint_4
  // CHECK:   [[bits_1:%[0-9]+]] = OpShiftLeftLogical %uint [[byte_1]] %uint_3
  // CHECK:    [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK:   [[uint_1:%[0-9]+]] = OpLoad %uint [[ptr_1]]
  // CHECK:  [[shift_1:%[0-9]+]] = OpShiftRightLogical %uint [[uint_1]] [[bits_1]]
  // CHECK: [[ushort_1:%[0-9]+]] = OpUConvert %ushort [[shift_1]]
  // CHECK:                   OpStore %u16 [[ushort_1]]
  uint16_t u16 = buf.Load<uint16_t>(tid.x);

  // CHECK:    [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK:   [[uint_2:%[0-9]+]] = OpLoad %uint [[ptr_2]]
  // CHECK:  [[shift_2:%[0-9]+]] = OpShiftRightLogical %uint [[uint_2]] {{%[0-9]+}}
  // CHECK: [[ushort_2:%[0-9]+]] = OpUConvert %ushort [[shift_2]]
  // CHECK:  [[short:%[0-9]+]] = OpBitcast %short [[ushort_2]]
  // CHECK:                   OpStore %i16 [[short]]
  int16_t i16 = buf.Load<int16_t>(tid.x);

  // CHECK:    [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK:   [[uint_3:%[0-9]+]] = OpLoad %uint [[ptr_3]]
  // CHECK:  [[shift_3:%[0-9]+]] = OpShiftRightLogical %uint [[uint_3]] {{%[0-9]+}}
  // CHECK: [[ushort_3:%[0-9]+]] = OpUConvert %ushort [[shift_3]]
  // CHECK:   [[half:%[0-9]+]] = OpBitcast %half [[ushort_3]]
  // CHECK:                   OpStore %f16 [[half]]
  float16_t f16 = buf.Load<float16_t>(tid.x);

  // ********* 32-bit scalar ********************

  // CHECK:  [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK: [[uint_4:%[0-9]+]] = OpLoad %uint [[ptr_4]]
  // CHECK:                 OpStore %u [[uint_4]]
  uint u = buf.Load<uint>(tid.x);

  // CHECK:  [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK: [[uint_5:%[0-9]+]] = OpLoad %uint [[ptr_6:%[0-9]+]]
  // CHECK:  [[int:%[0-9]+]] = OpBitcast %int [[uint_5]]
  // CHECK:                 OpStore %i [[int]]
  int i = buf.Load<int>(tid.x);

  // CHECK:   [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK:  [[uint_6:%[0-9]+]] = OpLoad %uint [[ptr_7]]
  // CHECK: [[float:%[0-9]+]] = OpBitcast %float [[uint_6]]
  // CHECK:                  OpStore %f [[float]]
  float f = buf.Load<float>(tid.x);

  // CHECK:  [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 {{%[0-9]+}}
  // CHECK: [[uint_7:%[0-9]+]] = OpLoad %uint [[ptr_8]]
  // CHECK: [[bool:%[0-9]+]] = OpINotEqual %bool [[uint_7]] %uint_0
  // CHECK:                 OpStore %b [[bool]]
  bool b = buf.Load<bool>(tid.x);

  // ********* 64-bit scalar ********************

// CHECK:    [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[addr:%[0-9]+]]
// CHECK:    [[word0:%[0-9]+]] = OpLoad %uint [[ptr_9]]
// CHECK:  [[newAddr:%[0-9]+]] = OpIAdd %uint [[addr]] %uint_1
// CHECK:   [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[newAddr]]
// CHECK:    [[word1:%[0-9]+]] = OpLoad %uint [[ptr_10]]
// CHECK:    [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0]] [[word1]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %ulong [[merge]]
// CHECK:                        OpStore %u64 [[val]]
  uint64_t u64 = buf.Load<uint64_t>(tid.x);

// CHECK:     [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[addr_0:%[0-9]+]]
// CHECK:    [[word0_0:%[0-9]+]] = OpLoad %uint [[ptr_11]]
// CHECK:  [[newAddr_0:%[0-9]+]] = OpIAdd %uint [[addr_0]] %uint_1
// CHECK:     [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[newAddr_0]]
// CHECK:    [[word1_0:%[0-9]+]] = OpLoad %uint [[ptr_12]]
// CHECK:      [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0_0]] [[word1_0]]
// CHECK:   [[val_long:%[0-9]+]] = OpBitcast %long [[merge]]
// CHECK:                          OpStore %i64 [[val_long]]
  int64_t i64 = buf.Load<int64_t>(tid.x);

// CHECK:      [[ptr_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[addr_1:%[0-9]+]]
// CHECK:     [[word0_1:%[0-9]+]] = OpLoad %uint [[ptr_13]]
// CHECK:   [[newAddr_1:%[0-9]+]] = OpIAdd %uint [[addr_1]] %uint_1
// CHECK:      [[ptr_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[newAddr_1]]
// CHECK:     [[word1_1:%[0-9]+]] = OpLoad %uint [[ptr_14]]
// CHECK:       [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[word0_1]] [[word1_1]]
// CHECK:  [[val_double:%[0-9]+]] = OpBitcast %double [[merge]]
// CHECK:                           OpStore %f64 [[val_double]]
  double f64 = buf.Load<double>(tid.x);

  // ********* array of scalars *****************


// CHECK:   [[index0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0:%[0-9]+]] %uint_2
// CHECK: [[byteOff0:%[0-9]+]] = OpUMod %uint [[addr0]] %uint_4
// CHECK:  [[bitOff0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff0]] %uint_3
// CHECK:   [[ptr_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index0]]
// CHECK:  [[word0_2:%[0-9]+]] = OpLoad %uint [[ptr_15]]
// CHECK:  [[shift_4:%[0-9]+]] = OpShiftRightLogical %uint [[word0_2]] [[bitOff0]]
// CHECK:     [[val0:%[0-9]+]] = OpUConvert %ushort [[shift_4]]
// CHECK:    [[addr1:%[0-9]+]] = OpIAdd %uint [[addr0]] %uint_2
// CHECK:   [[index1:%[0-9]+]] = OpShiftRightLogical %uint [[addr1]] %uint_2
// CHECK: [[byteOff1:%[0-9]+]] = OpUMod %uint [[addr1]] %uint_4
// CHECK:  [[bitOff1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff1]] %uint_3
// CHECK:   [[ptr_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index1]]
// CHECK:  [[word0_3:%[0-9]+]] = OpLoad %uint [[ptr_16]]
// CHECK: [[val1uint:%[0-9]+]] = OpShiftRightLogical %uint [[word0_3]] [[bitOff1]]
// CHECK:     [[val1:%[0-9]+]] = OpUConvert %ushort [[val1uint]]
// CHECK:    [[addr2:%[0-9]+]] = OpIAdd %uint [[addr1]] %uint_2
// CHECK:   [[index2:%[0-9]+]] = OpShiftRightLogical %uint [[addr2]] %uint_2
// CHECK: [[byteOff2:%[0-9]+]] = OpUMod %uint [[addr2]] %uint_4
// CHECK:  [[bitOff2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff2]] %uint_3
// CHECK:   [[ptr_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index2]]
// CHECK:  [[word1_2:%[0-9]+]] = OpLoad %uint [[ptr_17]]
// CHECK:  [[shift_5:%[0-9]+]] = OpShiftRightLogical %uint [[word1_2]] [[bitOff2]]
// CHECK:     [[val2:%[0-9]+]] = OpUConvert %ushort [[shift_5]]
// CHECK:     [[uArr:%[0-9]+]] = OpCompositeConstruct %_arr_ushort_uint_3 [[val0]] [[val1]] [[val2]]
// CHECK:                        OpStore %uArr [[uArr]]
  uint16_t uArr[3] = buf.Load<uint16_t[3]>(tid.x);

// CHECK:     [[index_1:%[0-9]+]] = OpShiftRightLogical %uint [[addr_2:%[0-9]+]] %uint_2
// CHECK:      [[ptr_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:   [[val0_uint:%[0-9]+]] = OpLoad %uint [[ptr_18]]
// CHECK:      [[val0_0:%[0-9]+]] = OpBitcast %int [[val0_uint]]
// CHECK:    [[newIndex:%[0-9]+]] = OpIAdd %uint [[index_1]] %uint_1
// CHECK:      [[ptr_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[newIndex]]
// CHECK:   [[val1_uint:%[0-9]+]] = OpLoad %uint [[ptr_19]]
// CHECK:      [[val1_0:%[0-9]+]] = OpBitcast %int [[val1_uint]]
// CHECK:        [[iArr:%[0-9]+]] = OpCompositeConstruct %_arr_int_uint_2 [[val0_0]] [[val1_0]]
// CHECK:                           OpStore %iArr [[iArr]]
  int iArr[2] = buf.Load<int[2]>(tid.x);

// CHECK:          [[index_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr_0:%[0-9]+]] %uint_2
// CHECK:           [[ptr_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
// CHECK:  [[val0_word0_uint:%[0-9]+]] = OpLoad %uint [[ptr_20]]
// CHECK:          [[index_1:%[0-9]+]] = OpIAdd %uint [[index_0]] %uint_1
// CHECK:           [[ptr_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:  [[val0_word1_uint:%[0-9]+]] = OpLoad %uint [[ptr_21]]
// CHECK:          [[index_2:%[0-9]+]] = OpIAdd %uint [[index_1]] %uint_1
// CHECK:            [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[val0_word0_uint]] [[val0_word1_uint]]
// CHECK:      [[val0_double:%[0-9]+]] = OpBitcast %double [[merge]]

// CHECK:           [[ptr_22:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:  [[val1_word0_uint:%[0-9]+]] = OpLoad %uint [[ptr_22]]
// CHECK:          [[index_3:%[0-9]+]] = OpIAdd %uint [[index_2]] %uint_1
// CHECK:           [[ptr_23:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:  [[val1_word1_uint:%[0-9]+]] = OpLoad %uint [[ptr_23]]
// CHECK:          [[index_4:%[0-9]+]] = OpIAdd %uint [[index_3]] %uint_1
// CHECK:            [[merge:%[0-9]+]] = OpCompositeConstruct %v2uint [[val1_word0_uint]] [[val1_word1_uint]]
// CHECK:      [[val1_double:%[0-9]+]] = OpBitcast %double [[merge]]
//
// CHECK:             [[fArr:%[0-9]+]] = OpCompositeConstruct %_arr_double_uint_2 [[val0_double]] [[val1_double]]
// CHECK:                                OpStore %fArr [[fArr]]
  double fArr[2] = buf.Load<double[2]>(tid.x);
}

