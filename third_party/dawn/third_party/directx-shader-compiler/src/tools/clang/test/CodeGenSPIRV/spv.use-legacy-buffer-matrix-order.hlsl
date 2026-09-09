// RUN: %dxc -T cs_6_2 -E main -fspv-use-legacy-buffer-matrix-order -fcgl  %s -spirv | FileCheck %s
//
// In this test, the default matrix order should be row major.
// We also check that the matrix elements are stored in the same order as
// they were loaded in.

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId)
{
// CHECK:[[index_0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0:%[0-9]+]] %uint_2
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_0]]
// CHECK:  [[word0:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:   [[val0:%[0-9]+]] = OpBitcast %int [[word0]]
// CHECK:[[index_1:%[0-9]+]] = OpIAdd %uint [[index_0]] %uint_1
// CHECK:    [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_1]]
// CHECK:  [[word1:%[0-9]+]] = OpLoad %uint [[ptr_0]]
// CHECK:   [[val1:%[0-9]+]] = OpBitcast %int [[word1_0:%[0-9]+]]
// CHECK:[[index_2:%[0-9]+]] = OpIAdd %uint [[index_1]] %uint_1
// CHECK:    [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_2]]
// CHECK:  [[word2:%[0-9]+]] = OpLoad %uint [[ptr_1]]
// CHECK:   [[val2:%[0-9]+]] = OpBitcast %int [[word2]]
// CHECK:[[index_3:%[0-9]+]] = OpIAdd %uint [[index_2]] %uint_1
// CHECK:    [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_3]]
// CHECK:  [[word3:%[0-9]+]] = OpLoad %uint [[ptr_2]]
// CHECK:   [[val3:%[0-9]+]] = OpBitcast %int [[word3]]
// CHECK:[[index_4:%[0-9]+]] = OpIAdd %uint [[index_3]] %uint_1
// CHECK:    [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_4]]
// CHECK:  [[word4:%[0-9]+]] = OpLoad %uint [[ptr_3]]
// CHECK:   [[val4:%[0-9]+]] = OpBitcast %int [[word4]]
// CHECK:[[index_5:%[0-9]+]] = OpIAdd %uint [[index_4]] %uint_1
// CHECK:    [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index_5]]
// CHECK:  [[word5:%[0-9]+]] = OpLoad %uint [[ptr_4]]
// CHECK:   [[val5:%[0-9]+]] = OpBitcast %int [[word5]]
// CHECK:   [[row0:%[0-9]+]] = OpCompositeConstruct %v2int [[val0]] [[val1]]
// CHECK:   [[row1:%[0-9]+]] = OpCompositeConstruct %v2int [[val2]] [[val3]]
// CHECK:   [[row2:%[0-9]+]] = OpCompositeConstruct %v2int [[val4]] [[val5]]
// CHECK:   [[mat0:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[row0]] [[row1]] [[row2]]
// CHECK:                   OpStore [[temp:%[a-zA-Z0-9_]+]] [[mat0]]
// CHECK:   [[mat1:%[0-9]+]] = OpLoad %_arr_v2int_uint_3 [[temp]]
// CHECK:  [[elem0:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 0 0
// CHECK:  [[elem1:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 0 1
// CHECK:  [[elem2:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 1 0
// CHECK:  [[elem3:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 1 1
// CHECK:  [[elem4:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 2 0
// CHECK:  [[elem5:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 2 1
// CHECK:   [[idx0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_0:%[0-9]+]] %uint_2
// CHECK:    [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx0]]
// CHECK:    [[val:%[0-9]+]] = OpBitcast %uint [[elem0]]
// CHECK:                   OpStore [[ptr_5]] [[val]]
// CHECK:   [[idx1:%[0-9]+]] = OpIAdd %uint [[idx0]] %uint_1
// CHECK:    [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx1]]
// CHECK:    [[val_0:%[0-9]+]] = OpBitcast %uint [[elem1]]
// CHECK:                   OpStore [[ptr_6]] [[val_0]]
// CHECK:   [[idx2:%[0-9]+]] = OpIAdd %uint [[idx1]] %uint_1
// CHECK:    [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx2]]
// CHECK:    [[val_1:%[0-9]+]] = OpBitcast %uint [[elem2]]
// CHECK:                   OpStore [[ptr_7]] [[val_1]]
// CHECK:   [[idx3:%[0-9]+]] = OpIAdd %uint [[idx2]] %uint_1
// CHECK:    [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx3]]
// CHECK:    [[val_2:%[0-9]+]] = OpBitcast %uint [[elem3]]
// CHECK:                   OpStore [[ptr_8]] [[val_2]]
// CHECK:   [[idx4:%[0-9]+]] = OpIAdd %uint [[idx3]] %uint_1
// CHECK:    [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx4]]
// CHECK:    [[val_3:%[0-9]+]] = OpBitcast %uint [[elem4]]
// CHECK:                   OpStore [[ptr_9]] [[val_3]]
// CHECK:   [[idx5:%[0-9]+]] = OpIAdd %uint [[idx4]] %uint_1
// CHECK:    [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx5]]
// CHECK:    [[val_4:%[0-9]+]] = OpBitcast %uint [[elem5]]
// CHECK:                   OpStore [[ptr_10]] [[val_4]]

  int3x2 i = buf.Load<int3x2>(tid.x);
  buf2.Store<int3x2>(tid.x, i);
}
