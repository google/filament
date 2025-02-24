// RUN: %dxc -T cs_6_2 -E main -fcgl  %s -spirv | FileCheck %s
//
// In this test, check that matrix order is preserved on a templated store.

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
// CHECK:   [[row0:%[0-9]+]] = OpCompositeConstruct %v2int [[val0]] [[val2]]
// CHECK:   [[row1:%[0-9]+]] = OpCompositeConstruct %v2int [[val1]] [[val3]]
// CHECK:   [[mat0:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_2 [[row0]] [[row1]]
// CHECK:                   OpStore [[temp:%[a-zA-Z0-9_]+]] [[mat0]]
// CHECK:   [[mat1:%[0-9]+]] = OpLoad %_arr_v2int_uint_2 [[temp]]
// CHECK:  [[elem0:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 0 0
// CHECK:  [[elem1:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 1 0
// CHECK:  [[elem2:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 0 1
// CHECK:  [[elem3:%[0-9]+]] = OpCompositeExtract %int [[mat1]] 1 1
// CHECK:   [[idx0:%[0-9]+]] = OpShiftRightLogical %uint [[addr0_0:%[0-9]+]] %uint_2
// CHECK:    [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx0]]
// CHECK:    [[val:%[0-9]+]] = OpBitcast %uint [[elem0]]
// CHECK:                   OpStore [[ptr_3]] [[val]]
// CHECK:   [[idx1:%[0-9]+]] = OpIAdd %uint [[idx0]] %uint_1
// CHECK:    [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx1]]
// CHECK:    [[val_0:%[0-9]+]] = OpBitcast %uint [[elem1]]
// CHECK:                   OpStore [[ptr_4]] [[val_0]]
// CHECK:   [[idx2:%[0-9]+]] = OpIAdd %uint [[idx1]] %uint_1
// CHECK:    [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx2]]
// CHECK:    [[val_1:%[0-9]+]] = OpBitcast %uint [[elem2]]
// CHECK:                   OpStore [[ptr_5]] [[val_1]]
// CHECK:   [[idx3:%[0-9]+]] = OpIAdd %uint [[idx2]] %uint_1
// CHECK:    [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[idx3]]
// CHECK:    [[val_2:%[0-9]+]] = OpBitcast %uint [[elem3]]
// CHECK:                   OpStore [[ptr_6]] [[val_2]]

  int2x2 i = buf.Load<int2x2>(tid.x);
  buf2.Store<int2x2>(tid.x, i);
}
