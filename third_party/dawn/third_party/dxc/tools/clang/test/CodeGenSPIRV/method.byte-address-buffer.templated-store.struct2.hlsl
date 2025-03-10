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

// a

// CHECK: OpStore %sArr
// CHECK:     [[tidx:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tid %int_0
// CHECK:  [[s0_addr:%[0-9]+]] = OpLoad %uint [[tidx]]
// CHECK:     [[sArr:%[0-9]+]] = OpLoad %_arr_S_uint_2 %sArr
// CHECK:    [[sArr0:%[0-9]+]] = OpCompositeExtract %S [[sArr]] 0
// CHECK:    [[sArr1:%[0-9]+]] = OpCompositeExtract %S [[sArr]] 1
// CHECK:     [[s0_a:%[0-9]+]] = OpCompositeExtract %half [[sArr0]] 0
// CHECK: [[s0_a_ind:%[0-9]+]] = OpShiftRightLogical %uint [[s0_addr]] %uint_2
// CHECK:  [[byteOff:%[0-9]+]] = OpUMod %uint [[s0_addr]] %uint_4
// CHECK:   [[bitOff:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff]] %uint_3
// CHECK:     [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[s0_a_ind]]
// CHECK: OpBitcast %ushort
// CHECK: OpUConvert %uint
// CHECK:  [[shifted:%[0-9]+]] = OpShiftLeftLogical %uint {{%[0-9]+}} [[bitOff]]
// CHECK:  [[maskOff:%[0-9]+]] = OpISub %uint %uint_16 [[bitOff]]
// CHECK:     [[mask:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOff]]
// CHECK: [[oldWord0:%[0-9]+]] = OpLoad %uint [[ptr0]]
// CHECK: [[maskWord:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord0]] [[mask]]
// CHECK: [[newWord0:%[0-9]+]] = OpBitwiseOr %uint [[maskWord]] [[shifted]]
// CHECK: OpStore [[ptr0]] [[newWord0]]

// e[0].x[0]

// CHECK:                     OpIAdd %uint [[s0_addr]] %uint_2
// CHECK:    [[eAddr:%[0-9]+]] = OpIAdd %uint [[s0_addr]] %uint_2
// CHECK:     [[s0_e:%[0-9]+]] = OpCompositeExtract %_arr_T_uint_2 [[sArr0]] 1
// CHECK:    [[s0_e0:%[0-9]+]] = OpCompositeExtract %T [[s0_e]] 0
// CHECK:    [[s0_e1:%[0-9]+]] = OpCompositeExtract %T [[s0_e]] 1
// CHECK:  [[s0_e0_x:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_2 [[s0_e0]] 0
// CHECK: [[s0_e0_x0:%[0-9]+]] = OpCompositeExtract %half [[s0_e0_x]] 0
// CHECK: [[s0_e0_x1:%[0-9]+]] = OpCompositeExtract %half [[s0_e0_x]] 1
// CHECK: [[s0_e_ind:%[0-9]+]] = OpShiftRightLogical %uint [[eAddr]] %uint_2
// CHECK:  [[byteOff_0:%[0-9]+]] = OpUMod %uint [[eAddr]] %uint_4
// CHECK:   [[bitOff_0:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_0]] %uint_3
// CHECK:     [[ptr0_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[s0_e_ind]]
// CHECK: OpBitcast %ushort [[s0_e0_x0]]
// CHECK: OpUConvert %uint
// CHECK:  [[shifted_0:%[0-9]+]] = OpShiftLeftLogical %uint {{%[0-9]+}} [[bitOff_0]]
// CHECK:  [[maskOff_0:%[0-9]+]] = OpISub %uint %uint_16 [[bitOff_0]]
// CHECK:     [[mask_0:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOff_0]]
// CHECK: [[oldWord0_0:%[0-9]+]] = OpLoad %uint [[ptr0_0]]
// CHECK: [[maskWord_0:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord0_0]] [[mask_0]]
// CHECK: [[newWord0_0:%[0-9]+]] = OpBitwiseOr %uint [[maskWord_0]] [[shifted_0]]
// CHECK:                     OpStore [[ptr0_0]] [[newWord0_0]]

// e[0].x[1]

// CHECK:[[s0_e0_x1_add:%[0-9]+]] = OpIAdd %uint [[eAddr]] %uint_2
// CHECK:[[s0_e0_x1_ind:%[0-9]+]] = OpShiftRightLogical %uint [[s0_e0_x1_add]] %uint_2
// CHECK:  [[byteOff_1:%[0-9]+]] = OpUMod %uint [[s0_e0_x1_add]] %uint_4
// CHECK:   [[bitOff_1:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_1]] %uint_3
// CHECK:     [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[s0_e0_x1_ind]]
// CHECK: OpBitcast %ushort [[s0_e0_x1]]
// CHECK: OpUConvert %uint
// CHECK:  [[shifted_1:%[0-9]+]] = OpShiftLeftLogical %uint {{%[0-9]+}} [[bitOff_1]]
// CHECK:  [[maskOff_1:%[0-9]+]] = OpISub %uint %uint_16 [[bitOff_1]]
// CHECK:     [[mask_1:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOff_1]]
// CHECK: [[oldWord1:%[0-9]+]] = OpLoad %uint [[ptr1]]
// CHECK: [[maskWord_1:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord1]] [[mask_1]]
// CHECK: [[newWord1:%[0-9]+]] = OpBitwiseOr %uint [[maskWord_1]] [[shifted_1]]
// CHECK:                     OpStore [[ptr1]] [[newWord1]]

// e[1].x[0]

// CHECK:   [[e1Addr:%[0-9]+]] = OpIAdd %uint [[eAddr]] %uint_4
// CHECK:  [[s0_e1_x:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_2 [[s0_e1]] 0
// CHECK: [[s0_e1_x0:%[0-9]+]] = OpCompositeExtract %half [[s0_e1_x]] 0
// CHECK: [[s0_e1_x1:%[0-9]+]] = OpCompositeExtract %half [[s0_e1_x]] 1
// CHECK:[[s0_e1_x0_ind:%[0-9]+]] = OpShiftRightLogical %uint [[e1Addr]] %uint_2
// CHECK:  [[byteOff_2:%[0-9]+]] = OpUMod %uint [[e1Addr]] %uint_4
// CHECK:   [[bitOff_2:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_2]] %uint_3
// CHECK:     [[ptr1_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[s0_e1_x0_ind]]
// CHECK: OpBitcast %ushort [[s0_e1_x0]]
// CHECK: OpUConvert %uint
// CHECK:  [[shifted_2:%[0-9]+]] = OpShiftLeftLogical %uint {{%[0-9]+}} [[bitOff_2]]
// CHECK:  [[maskOff_2:%[0-9]+]] = OpISub %uint %uint_16 [[bitOff_2]]
// CHECK:     [[mask_2:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOff_2]]
// CHECK: [[oldWord1_0:%[0-9]+]] = OpLoad %uint [[ptr1_0]]
// CHECK: [[maskWord_2:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord1_0]] [[mask_2]]
// CHECK: [[newWord1_0:%[0-9]+]] = OpBitwiseOr %uint [[maskWord_2]] [[shifted_2]]
// CHECK:                     OpStore [[ptr1_0]] [[newWord1_0]]

// e[1].x[1]

// CHECK:[[s0_e1_x1_add:%[0-9]+]] = OpIAdd %uint [[e1Addr]] %uint_2
// CHECK:[[s0_e1_x1_ind:%[0-9]+]] = OpShiftRightLogical %uint [[s0_e1_x1_add]] %uint_2
// CHECK:  [[byteOff_3:%[0-9]+]] = OpUMod %uint [[s0_e1_x1_add]] %uint_4
// CHECK:   [[bitOff_3:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_3]] %uint_3
// CHECK:     [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[s0_e1_x1_ind]]
// CHECK: OpBitcast %ushort [[s0_e1_x1]]
// CHECK: OpUConvert %uint
// CHECK:                     OpStore [[ptr2]] {{%[0-9]+}}

// a

// CHECK:  [[s1_addr:%[0-9]+]] = OpIAdd %uint [[s0_addr]] %uint_10
// CHECK:     [[s1_a:%[0-9]+]] = OpCompositeExtract %half [[sArr1]] 0
// CHECK: [[s1_a_ind:%[0-9]+]] = OpShiftRightLogical %uint [[s1_addr]] %uint_2
// CHECK:  [[byteOff_4:%[0-9]+]] = OpUMod %uint [[s1_addr]] %uint_4
// CHECK:   [[bitOff_4:%[0-9]+]] = OpShiftLeftLogical %uint [[byteOff_4]] %uint_3
// CHECK:     [[ptr2_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[s1_a_ind]]
// CHECK: OpBitcast %ushort [[s1_a]]
// CHECK: OpUConvert %uint
// CHECK:  [[shifted_3:%[0-9]+]] = OpShiftLeftLogical %uint {{%[0-9]+}} [[bitOff_4]]
// CHECK:  [[maskOff_3:%[0-9]+]] = OpISub %uint %uint_16 [[bitOff_4]]
// CHECK:     [[mask_3:%[0-9]+]] = OpShiftLeftLogical %uint %uint_65535 [[maskOff_3]]
// CHECK: [[oldWord2:%[0-9]+]] = OpLoad %uint [[ptr2_0]]
// CHECK: [[maskWord_3:%[0-9]+]] = OpBitwiseAnd %uint [[oldWord2]] [[mask_3]]
// CHECK: [[newWord2:%[0-9]+]] = OpBitwiseOr %uint [[maskWord_3]] [[shifted_3]]
// CHECK:                     OpStore [[ptr2_0]] [[newWord2]]

// e[0].x[0]
// e[0].x[1]

// CHECK:    [[eAddr_0:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_2
// CHECK:    [[eAddr_1:%[0-9]+]] = OpIAdd %uint [[s1_addr]] %uint_2
// CHECK:     [[s1_e:%[0-9]+]] = OpCompositeExtract %_arr_T_uint_2 [[sArr1]] 1
// CHECK:    [[s1_e0:%[0-9]+]] = OpCompositeExtract %T [[s1_e]] 0
// CHECK:    [[s1_e1:%[0-9]+]] = OpCompositeExtract %T [[s1_e]] 1
// CHECK:  [[s1_e0_x:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_2 [[s1_e0]] 0
// CHECK: [[s1_e0_x0:%[0-9]+]] = OpCompositeExtract %half [[s1_e0_x]] 0
// CHECK: [[s1_e0_x1:%[0-9]+]] = OpCompositeExtract %half [[s1_e0_x]] 1
// CHECK: OpBitcast %ushort [[s1_e0_x0]]
// CHECK: OpUConvert %uint
// CHECK:[[index:%[0-9]+]] = OpShiftRightLogical %uint {{%[0-9]+}}
// CHECK: [[ptr3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[index]]
// CHECK: OpBitcast %ushort [[s1_e0_x1]]
// CHECK: OpUConvert %uint
// CHECK: OpShiftLeftLogical %uint {{%[0-9]+}} {{%[0-9]+}}
// CHECK: OpBitwiseOr %uint
// CHECK: OpStore [[ptr3]] {{%[0-9]+}}

// e[1].x[0]
// e[1].x[1]

// CHECK:   [[e1Addr_0:%[0-9]+]] = OpIAdd %uint [[eAddr_1]] %uint_4
// CHECK:  [[s1_e1_x:%[0-9]+]] = OpCompositeExtract %_arr_half_uint_2 [[s1_e1]] 0
// CHECK: [[s1_e1_x0:%[0-9]+]] = OpCompositeExtract %half [[s1_e1_x]] 0
// CHECK: [[s1_e1_x1:%[0-9]+]] = OpCompositeExtract %half [[s1_e1_x]] 1
// CHECK: OpBitcast %ushort
// CHECK: OpUConvert %uint
// CHECK:[[index_0:%[0-9]+]] = OpShiftRightLogical %uint {{%[0-9]+}}
// CHECK: [[ptr4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %buf2 %uint_0 [[index_0]]
// CHECK: OpBitcast %ushort
// CHECK: OpUConvert %uint
// CHECK: OpShiftLeftLogical %uint {{%[0-9]+}} {{%[0-9]+}}
// CHECK: OpBitwiseOr %uint
// CHECK: OpStore [[ptr4]] {{%[0-9]+}}
