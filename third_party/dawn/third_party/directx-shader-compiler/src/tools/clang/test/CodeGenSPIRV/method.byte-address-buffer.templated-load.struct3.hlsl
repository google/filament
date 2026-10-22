// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

struct T {
  float16_t x[2];
};

struct S {
  float16_t a;
  T e[2];
};

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId) {
  S sArr[2] = buf.Load<S[2]>(tid.x);
  buf2.Store<S[2]>(tid.x, sArr);
}

// Note: the DX layout tightly packs all members of S and its sub-structures.
// It stores elements at the following byte offsets:
// 0, 2, 4, 6, 8, 10, 12, 14, 16, 18
//
//                              |-----------------------|
// address 0:                   |     a     | e[0].x[0] |
//                              |-----------------------|
// address 1 (byte offset 4):   | e[0].x[1] | e[1].x[0] |
//                              |-----------------------|
// address 2 (byte offset 8):   | e[1].x[1] |     a     |
//                              |-----------------------|
// address 3 (byte offset 12)   | e[0].x[0] | e[0].x[1] |
//                              |-----------------------|
// address 4 (byte offset 16)   | e[1].x[0] | e[1].x[1] |
//                              |-----------------------|
//

// CHECK:      [[tidx_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tid %int_0
// CHECK:  [[base_address:%[0-9]+]] = OpLoad %uint [[tidx_ptr]]
// CHECK:       [[a_index:%[0-9]+]] = OpShiftRightLogical %uint [[base_address]] %uint_2
// CHECK:    [[byteOffset:%[0-9]+]] = OpUMod %uint [[base_address]] %uint_4
// CHECK:     [[bitOffset:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset]] %uint_3
// CHECK:          [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[a_index]]
// CHECK:         [[word0:%[0-9]+]] = OpLoad %uint [[ptr0]]
// CHECK:       [[shifted:%[0-9]+]] = OpShiftRightLogical %uint [[word0]] [[bitOffset]]
// CHECK:      [[word0u16:%[0-9]+]] = OpUConvert %ushort [[shifted]]
// CHECK:             [[a:%[0-9]+]] = OpBitcast %half [[word0u16]]
// CHECK:    [[e0_address:%[0-9]+]] = OpIAdd %uint [[base_address]] %uint_2
// CHECK:    [[e0_address_0:%[0-9]+]] = OpIAdd %uint [[base_address]] %uint_2
// CHECK:      [[e0_index:%[0-9]+]] = OpShiftRightLogical %uint [[e0_address_0]] %uint_2
// CHECK:    [[byteOffset_0:%[0-9]+]] = OpUMod %uint [[e0_address_0]] %uint_4
// CHECK:     [[bitOffset_0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_0]] %uint_3
// CHECK:          [[ptr0_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[e0_index]]
// CHECK:         [[word0_0:%[0-9]+]] = OpLoad %uint [[ptr0_0]]
// CHECK:    [[word0upper:%[0-9]+]] = OpShiftRightLogical %uint [[word0_0]] [[bitOffset_0]]
// CHECK: [[word0upperu16:%[0-9]+]] = OpUConvert %ushort [[word0upper]]
// CHECK:           [[x_0:%[0-9]+]] = OpBitcast %half [[word0upperu16]]
// CHECK:    [[x1_address:%[0-9]+]] = OpIAdd %uint [[e0_address_0]] %uint_2
// CHECK:      [[x1_index:%[0-9]+]] = OpShiftRightLogical %uint [[x1_address]] %uint_2
// CHECK:    [[byteOffset_1:%[0-9]+]] = OpUMod %uint [[x1_address]] %uint_4
// CHECK:     [[bitOffset_1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_1]] %uint_3
// CHECK:          [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[x1_index]]
// CHECK:         [[word1:%[0-9]+]] = OpLoad %uint [[ptr1]]
// CHECK:       [[shifted_0:%[0-9]+]] = OpShiftRightLogical %uint [[word1]] [[bitOffset_1]]
// CHECK:      [[word1u16:%[0-9]+]] = OpUConvert %ushort [[shifted_0]]
// CHECK:           [[x_1:%[0-9]+]] = OpBitcast %half [[word1u16]]
// CHECK:             [[x:%[0-9]+]] = OpCompositeConstruct %_arr_half_uint_2 [[x_0]] [[x_1]]
// CHECK:    [[e1_address:%[0-9]+]] = OpIAdd %uint [[e0_address_0]] %uint_4
// CHECK:           [[e_0:%[0-9]+]] = OpCompositeConstruct %T [[x]]
// CHECK:      [[e1_index:%[0-9]+]] = OpShiftRightLogical %uint [[e1_address]] %uint_2
// CHECK:    [[byteOffset_2:%[0-9]+]] = OpUMod %uint [[e1_address]] %uint_4
// CHECK:     [[bitOffset_2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_2]] %uint_3
// CHECK:          [[ptr1_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[e1_index]]
// CHECK:         [[word1_0:%[0-9]+]] = OpLoad %uint [[ptr1_0]]
// CHECK:    [[word1upper:%[0-9]+]] = OpShiftRightLogical %uint [[word1_0]] [[bitOffset_2]]
// CHECK: [[word1upperu16:%[0-9]+]] = OpUConvert %ushort [[word1upper]]
// CHECK:           [[x_1:%[0-9]+]] = OpBitcast %half [[word1upperu16]]
// CHECK:    [[x1_address_0:%[0-9]+]] = OpIAdd %uint [[e1_address]] %uint_2
// CHECK:      [[x1_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[x1_address_0]] %uint_2
// CHECK:    [[byteOffset_3:%[0-9]+]] = OpUMod %uint [[x1_address_0]] %uint_4
// CHECK:     [[bitOffset_3:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_3]] %uint_3
// CHECK:          [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[x1_index_0]]
// CHECK:         [[word2:%[0-9]+]] = OpLoad %uint [[ptr2]]
// CHECK:       [[shifted_1:%[0-9]+]] = OpShiftRightLogical %uint [[word2]] [[bitOffset_3]]
// CHECK:      [[word2u16:%[0-9]+]] = OpUConvert %ushort [[shifted_1]]
// CHECK:           [[x_2:%[0-9]+]] = OpBitcast %half [[word2u16]]
// CHECK:             [[x_0:%[0-9]+]] = OpCompositeConstruct %_arr_half_uint_2 [[x_1]] [[x_2]]
// CHECK:           [[e_1:%[0-9]+]] = OpCompositeConstruct %T [[x_0]]
// CHECK:             [[e:%[0-9]+]] = OpCompositeConstruct %_arr_T_uint_2 [[e_0]] [[e_1]]
// CHECK:    [[s1_address:%[0-9]+]] = OpIAdd %uint [[base_address]] %uint_10
// CHECK:           [[s_0:%[0-9]+]] = OpCompositeConstruct %S [[a]] [[e]]
//
// Now start with the second 'S' object
//
// CHECK:      [[s1_index:%[0-9]+]] = OpShiftRightLogical %uint [[s1_address]] %uint_2
// CHECK:    [[byteOffset_4:%[0-9]+]] = OpUMod %uint [[s1_address]] %uint_4
// CHECK:     [[bitOffset_4:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOffset_4]] %uint_3
// CHECK:          [[ptr2_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[s1_index]]
// CHECK:         [[word2_0:%[0-9]+]] = OpLoad %uint [[ptr2_0]]
// CHECK:  [[word2upper16:%[0-9]+]] = OpShiftRightLogical %uint [[word2_0]] [[bitOffset_4]]
// CHECK: [[word2upperu16:%[0-9]+]] = OpUConvert %ushort [[word2upper16]]
// CHECK:             [[a_0:%[0-9]+]] = OpBitcast %half [[word2upperu16]]
// CHECK:    [[e0_address_1:%[0-9]+]] = OpIAdd %uint [[s1_address]] %uint_2
// CHECK:    [[e0_address_2:%[0-9]+]] = OpIAdd %uint [[s1_address]] %uint_2
// CHECK:      [[e0_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[e0_address_2]] %uint_2
// CHECK:               {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[e0_index_0]]
// CHECK:    [[x1_address_1:%[0-9]+]] = OpIAdd %uint [[e0_address_2]] %uint_2
// CHECK:      [[x1_index_1:%[0-9]+]] = OpShiftRightLogical %uint [[x1_address_1]] %uint_2
// CHECK:               {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[x1_index_1]]
// CHECK:                          OpCompositeConstruct %_arr_half_uint_2
// CHECK:    [[e1_address_0:%[0-9]+]] = OpIAdd %uint [[e0_address_2]] %uint_4
// CHECK:                          OpCompositeConstruct %T
// CHECK:      [[e1_index_0:%[0-9]+]] = OpShiftRightLogical %uint [[e1_address_0]] %uint_
// CHECK:                          OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[e1_index_0]]
// CHECK:    [[x1_address_2:%[0-9]+]] = OpIAdd %uint [[e1_address_0]] %uint_2
// CHECK:      [[x1_index_2:%[0-9]+]] = OpShiftRightLogical %uint [[x1_address_2]] %uint_2
// CHECK:                          OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[x1_index_2]]
// CHECK:                          OpCompositeConstruct %_arr_half_uint_2
// CHECK:                          OpCompositeConstruct %T
// CHECK:                          OpCompositeConstruct %_arr_T_uint_2
// CHECK:                          OpCompositeConstruct %S
// CHECK:                          OpCompositeConstruct %_arr_S_uint_2
// CHECK:                          OpStore %sArr {{%[0-9]+}}
