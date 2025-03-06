// RUN: %dxc -T cs_6_6 -E main -fcgl  %s -spirv | FileCheck %s

groupshared int dest_i;
groupshared uint dest_u;
groupshared float dest_f;

RWBuffer<uint> buff;
RWBuffer<uint> getDest() {
  return buff;
}

[numthreads(1,1,1)]
void main()
{
  uint original_u_val;
  int original_i_val;
  float original_f_val;

  int   val1;
  int   val2;
  float val_f1;

  //////////////////////////////////////////////////////////////////////////
  ///////      Test all Interlocked* functions on primitive types     //////
  ///////                Only int and uint are allowd                 //////
  //////////////////////////////////////////////////////////////////////////

// CHECK:      [[val1_27:%[0-9]+]] = OpLoad %int %val1
// CHECK-NEXT: [[iadd27:%[0-9]+]] = OpAtomicIAdd %int %dest_i %uint_2 %uint_0 [[val1_27]]
// CHECK-NEXT:                   OpStore %original_i_val [[iadd27]]
  InterlockedAdd(dest_i, val1, original_i_val);

// CHECK:      [[buff:%[0-9]+]] = OpFunctionCall %type_buffer_image %getDest
// CHECK-NEXT: OpStore %temp_var_RWBuffer [[buff]]
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %temp_var_RWBuffer %uint_0 %uint_0
// CHECK-NEXT: [[load_28:%[0-9]+]] = OpLoad %int %val1
// CHECK-NEXT: [[val1_28:%[0-9]+]] = OpBitcast %uint [[load_28]]
// CHECK-NEXT: [[iadd28:%[0-9]+]] = OpAtomicIAdd %uint [[ptr]] %uint_1 %uint_0 [[val1_28]]
// CHECK-NEXT: [[iadd28_2:%[0-9]+]] = OpBitcast %int [[iadd28]]
// CHECK-NEXT:       OpStore %original_i_val [[iadd28_2]]
  InterlockedAdd(getDest()[0], val1, original_i_val);

// CHECK:      [[and28:%[0-9]+]] = OpAtomicAnd %uint %dest_u %uint_2 %uint_0 %uint_10
// CHECK-NEXT:                  OpStore %original_u_val [[and28]]
  InterlockedAnd(dest_u, 10,  original_u_val);

// CHECK:       [[uint10:%[0-9]+]] = OpBitcast %int %uint_10
// CHECK-NEXT: [[asmax29:%[0-9]+]] = OpAtomicSMax %int %dest_i %uint_2 %uint_0 [[uint10]]
// CHECK-NEXT:                    OpStore %original_i_val [[asmax29]]
  InterlockedMax(dest_i, 10,  original_i_val);

// CHECK:      [[val30:%[0-9]+]] = OpBitcast %uint %int_n5
// CHECK-NEXT: [[aumax:%[0-9]+]] = OpAtomicUMax %uint %dest_u %uint_2 %uint_0 [[val30]]
// CHECK-NEXT: [[res30:%[0-9]+]] = OpBitcast %int [[aumax]]
// CHECK-NEXT:                     OpStore %original_i_val [[res30]]
  InterlockedMax(dest_u, -5,  original_i_val);

// CHECK:      [[umin30:%[0-9]+]] = OpAtomicUMin %uint %dest_u %uint_2 %uint_0 %uint_10
// CHECK-NEXT:                   OpStore %original_u_val [[umin30]]
  InterlockedMin(dest_u, 10,  original_u_val);

// CHECK:      [[val31:%[0-9]+]] = OpBitcast %int %uint_5
// CHECK-NEXT: [[asmin:%[0-9]+]] = OpAtomicSMin %int %dest_i %uint_2 %uint_0 [[val31]]
// CHECK-NEXT: [[res31:%[0-9]+]] = OpBitcast %uint [[asmin]]
// CHECK-NEXT:                     OpStore %original_u_val [[res31]]
  InterlockedMin(dest_i, 5u,  original_u_val);

// CHECK:      [[val2_31:%[0-9]+]] = OpLoad %int %val2
// CHECK-NEXT:   [[or31:%[0-9]+]] = OpAtomicOr %int %dest_i %uint_2 %uint_0 [[val2_31]]
// CHECK-NEXT:                   OpStore %original_i_val [[or31]]
  InterlockedOr (dest_i, val2, original_i_val);

// CHECK:      [[xor32:%[0-9]+]] = OpAtomicXor %uint %dest_u %uint_2 %uint_0 %uint_10
// CHECK-NEXT:                  OpStore %original_u_val [[xor32]]
  InterlockedXor(dest_u, 10,  original_u_val);

// CHECK:      [[val1_33:%[0-9]+]] = OpLoad %int %val1
// CHECK-NEXT: [[val2_33:%[0-9]+]] = OpLoad %int %val2
// CHECK-NEXT:        {{%[0-9]+}} = OpAtomicCompareExchange %int %dest_i %uint_2 %uint_0 %uint_0 [[val2_33]] [[val1_33]]
  InterlockedCompareStore(dest_i, val1, val2);

// CHECK:      [[ace34:%[0-9]+]] = OpAtomicCompareExchange %uint %dest_u %uint_2 %uint_0 %uint_0 %uint_20 %uint_15
// CHECK-NEXT:                  OpStore %original_u_val [[ace34]]
  InterlockedCompareExchange(dest_u, 15, 20, original_u_val);

// CHECK:      [[val2_35:%[0-9]+]] = OpLoad %int %val2
// CHECK-NEXT:  [[ace35:%[0-9]+]] = OpAtomicExchange %int %dest_i %uint_2 %uint_0 [[val2_35]]
// CHECK-NEXT:                   OpStore %original_i_val [[ace35]]
  InterlockedExchange(dest_i, val2, original_i_val);

// CHECK:      [[val_f:%[0-9]+]] = OpLoad %float %val_f1
// CHECK-NEXT:  [[ace36:%[0-9]+]] = OpAtomicExchange %float %dest_f %uint_2 %uint_0 [[val_f]]
// CHECK-NEXT:                   OpStore %original_f_val [[ace36]]
  InterlockedExchange(dest_f, val_f1, original_f_val);
}
