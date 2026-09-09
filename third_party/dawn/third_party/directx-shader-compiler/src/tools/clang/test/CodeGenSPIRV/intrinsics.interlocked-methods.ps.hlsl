// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s -check-prefix=CHECK -check-prefix=GLSL450
// RUN: %dxc -T ps_6_0 -E main -fcgl -fspv-use-vulkan-memory-model -fspv-target-env=vulkan1.1 %s -spirv | FileCheck %s -check-prefix=CHECK -check-prefix=VULKAN

RWTexture1D <int>   g_tTex1di1;
RWTexture1D <uint>  g_tTex1du1;

RWTexture2D <int>   g_tTex2di1;
RWTexture2D <uint>  g_tTex2du1;

RWTexture3D <int>   g_tTex3di1;
RWTexture3D <uint>  g_tTex3du1;

RWTexture1DArray <int>   g_tTex1di1a;
RWTexture1DArray <uint>  g_tTex1du1a;

RWTexture2DArray <int>   g_tTex2di1a;
RWTexture2DArray <uint>  g_tTex2du1a;

RWBuffer <int>   g_tBuffI;
RWBuffer <uint>  g_tBuffU;

RWStructuredBuffer<uint> g_tRWBuffU;

void main()
{
  uint out_u1;
  int out_i1;

  uint  u1;
  uint2 u2;
  uint3 u3;
  uint  u1b;
  uint  u1c;

  int   i1;
  int2  i2;
  int3  i3;
  int   i1b;
  int   i1c;

  ////////////////////////////////////////////////////////////////////
  /////   Test that type mismatches are resolved correctly    ////////
  ////////////////////////////////////////////////////////////////////

// CHECK:         [[idx0:%[0-9]+]] = OpLoad %uint %u1
// CHECK-NEXT:    [[ptr0:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 [[idx0]] %uint_0
// CHECK-NEXT:    [[i1_0:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT:   [[iadd0:%[0-9]+]] = OpAtomicIAdd %int [[ptr0]] %uint_1 %uint_0 [[i1_0]]
// VULKAN-NEXT:   [[iadd0:%[0-9]+]] = OpAtomicIAdd %int [[ptr0]] %uint_5 %uint_0 [[i1_0]]
// CHECK-NEXT: [[iadd0_u:%[0-9]+]] = OpBitcast %uint [[iadd0]]
// CHECK-NEXT:                    OpStore %out_u1 [[iadd0_u]]
  InterlockedAdd(g_tTex1di1[u1], i1, out_u1); // Addition result must be cast to uint before being written to out_u1


// CHECK:        [[ptr1:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:   [[u1_1:%[0-9]+]] = OpLoad %uint %u1
// CHECK-NEXT: [[u1_int:%[0-9]+]] = OpBitcast %int [[u1_1]]
// GLSL450-NEXT:  [[iadd1:%[0-9]+]] = OpAtomicIAdd %int [[ptr1]] %uint_1 %uint_0 [[u1_int]]
// VULKAN-NEXT:  [[iadd1:%[0-9]+]] = OpAtomicIAdd %int [[ptr1]] %uint_5 %uint_0 [[u1_int]]
// CHECK-NEXT:                   OpStore %out_i1 [[iadd1]]
  InterlockedAdd(g_tTex1di1[u1], u1, out_i1); // u1 should be cast to int before being passed to addition instruction

// CHECK:         [[ptr2:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:    [[i1_2:%[0-9]+]] = OpLoad %int %i1
// CHECK-NEXT: [[i1_uint:%[0-9]+]] = OpBitcast %uint [[i1_2]]
// GLSL450-NEXT:   [[iadd2:%[0-9]+]] = OpAtomicIAdd %uint [[ptr2]] %uint_1 %uint_0 [[i1_uint]]
// VULKAN-NEXT:   [[iadd2:%[0-9]+]] = OpAtomicIAdd %uint [[ptr2]] %uint_5 %uint_0 [[i1_uint]]
// CHECK-NEXT:                    OpStore %out_u1 [[iadd2]]
  InterlockedAdd(g_tTex1du1[u1], i1, out_u1); // i1 should be cast to uint before being passed to addition instruction

// CHECK:           [[ptr3:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:      [[u1_3:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT:     [[iadd3:%[0-9]+]] = OpAtomicIAdd %uint [[ptr3]] %uint_1 %uint_0 [[u1_3]]
// VULKAN-NEXT:     [[iadd3:%[0-9]+]] = OpAtomicIAdd %uint [[ptr3]] %uint_5 %uint_0 [[u1_3]]
// CHECK-NEXT: [[iadd3_int:%[0-9]+]] = OpBitcast %int [[iadd3]]
// CHECK-NEXT:                      OpStore %out_i1 [[iadd3_int]]
  InterlockedAdd(g_tTex1du1[u1], u1, out_i1); // Addition result must be cast to int before being written to out_i1


// CHECK:           [[ptr4:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:     [[u1b_4:%[0-9]+]] = OpLoad %uint %u1b
// CHECK-NEXT: [[u1b_4_int:%[0-9]+]] = OpBitcast %int [[u1b_4]]
// CHECK-NEXT:     [[i1c_4:%[0-9]+]] = OpLoad %int %i1c
// GLSL450-NEXT:      [[ace4:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr4]] %uint_1 %uint_0 %uint_0 [[i1c_4]] [[u1b_4_int]]
// VULKAN-NEXT:      [[ace4:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr4]] %uint_5 %uint_0 %uint_0 [[i1c_4]] [[u1b_4_int]]
// CHECK-NEXT:                      OpStore %out_i1 [[ace4]]
  InterlockedCompareExchange(g_tTex1di1[u1], u1b, i1c, out_i1); // u1b should first be cast to int


// CHECK:           [[ptr5:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:     [[i1b_5:%[0-9]+]] = OpLoad %int %i1b
// CHECK-NEXT:     [[u1c_5:%[0-9]+]] = OpLoad %uint %u1c
// CHECK-NEXT: [[u1c_5_int:%[0-9]+]] = OpBitcast %int [[u1c_5]]
// GLSL450-NEXT:      [[ace5:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr5]] %uint_1 %uint_0 %uint_0 [[u1c_5_int]] [[i1b_5]]
// VULKAN-NEXT:      [[ace5:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr5]] %uint_5 %uint_0 %uint_0 [[u1c_5_int]] [[i1b_5]]
// CHECK-NEXT:                      OpStore %out_i1 [[ace5]]
  InterlockedCompareExchange(g_tTex1di1[u1], i1b, u1c, out_i1); // u1c should first be cast to int

// CHECK:           [[ptr6:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:     [[i1b_6:%[0-9]+]] = OpLoad %int %i1b
// CHECK-NEXT:     [[i1c_6:%[0-9]+]] = OpLoad %int %i1c
// GLSL450-NEXT:      [[ace6:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr6]] %uint_1 %uint_0 %uint_0 [[i1c_6]] [[i1b_6]]
// VULKAN-NEXT:      [[ace6:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr6]] %uint_5 %uint_0 %uint_0 [[i1c_6]] [[i1b_6]]
// CHECK-NEXT: [[ace6_uint:%[0-9]+]] = OpBitcast %uint [[ace6]]
// CHECK-NEXT:                      OpStore %out_u1 [[ace6_uint]]
  InterlockedCompareExchange(g_tTex1di1[u1], i1b, i1c, out_u1); // original value must be cast to uint before being written to out_u1

// CHECK:            [[ptr7:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:      [[u1b_7:%[0-9]+]] = OpLoad %uint %u1b
// CHECK-NEXT:      [[i1c_7:%[0-9]+]] = OpLoad %int %i1c
// CHECK-NEXT: [[i1c_7_uint:%[0-9]+]] = OpBitcast %uint [[i1c_7]]
// GLSL450-NEXT:       [[ace7:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr7]] %uint_1 %uint_0 %uint_0 [[i1c_7_uint]] [[u1b_7]]
// VULKAN-NEXT:       [[ace7:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr7]] %uint_5 %uint_0 %uint_0 [[i1c_7_uint]] [[u1b_7]]
// CHECK-NEXT:                       OpStore %out_u1 [[ace7]]
  InterlockedCompareExchange(g_tTex1du1[u1], u1b, i1c, out_u1); // i1c should first be cast to uint


// CHECK:            [[ptr8:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:      [[i1b_8:%[0-9]+]] = OpLoad %int %i1b
// CHECK-NEXT: [[i1b_8_uint:%[0-9]+]] = OpBitcast %uint [[i1b_8]]
// CHECK-NEXT:      [[u1c_8:%[0-9]+]] = OpLoad %uint %u1c
// GLSL450-NEXT:       [[ace8:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr8]] %uint_1 %uint_0 %uint_0 [[u1c_8]] [[i1b_8_uint]]
// VULKAN-NEXT:       [[ace8:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr8]] %uint_5 %uint_0 %uint_0 [[u1c_8]] [[i1b_8_uint]]
// CHECK-NEXT:                       OpStore %out_u1 [[ace8]]
  InterlockedCompareExchange(g_tTex1du1[u1], i1b, u1c, out_u1); // i1b should first be cast to uint


// CHECK:          [[ptr9:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:    [[u1b_9:%[0-9]+]] = OpLoad %uint %u1b
// CHECK-NEXT:    [[u1c_9:%[0-9]+]] = OpLoad %uint %u1c
// GLSL450-NEXT:     [[ace9:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr9]] %uint_1 %uint_0 %uint_0 [[u1c_9]] [[u1b_9]]
// VULKAN-NEXT:     [[ace9:%[0-9]+]] = OpAtomicCompareExchange %uint [[ptr9]] %uint_5 %uint_0 %uint_0 [[u1c_9]] [[u1b_9]]
// CHECK-NEXT: [[ace9_int:%[0-9]+]] = OpBitcast %int [[ace9]]
// CHECK-NEXT:                     OpStore %out_i1 [[ace9_int]]
  InterlockedCompareExchange(g_tTex1du1[u1], u1b, u1c, out_i1); // original value must be cast to int before being written to out_i1


//CHECK:             [[ptr10:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
//CHECK-NEXT:        [[u1_10:%[0-9]+]] = OpLoad %uint %u1
//CHECK-NEXT:    [[u1_10_int:%[0-9]+]] = OpBitcast %int [[u1_10]]
//GLSL450-NEXT:      [[asmax10:%[0-9]+]] = OpAtomicSMax %int [[ptr10]] %uint_1 %uint_0 [[u1_10_int]]
//VULKAN-NEXT:      [[asmax10:%[0-9]+]] = OpAtomicSMax %int [[ptr10]] %uint_5 %uint_0 [[u1_10_int]]
//CHECK-NEXT: [[asmax10_uint:%[0-9]+]] = OpBitcast %uint [[asmax10]]
//CHECK-NEXT:                         OpStore %out_u1 [[asmax10_uint]]
  // u1 should be cast to int first.
  // AtomicSMax should be performed.
  // Result should be cast to uint before being written to out_u1.
  InterlockedMax(g_tTex1di1[u1], u1, out_u1);


// CHECK:      [[ptr11:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1_11:%[0-9]+]] = OpLoad %int %i1
// CHECK-NEXT: [[i1_11_uint:%[0-9]+]] = OpBitcast %uint [[i1_11]]
// GLSL450-NEXT: [[aumin11:%[0-9]+]] = OpAtomicUMin %uint [[ptr11]] %uint_1 %uint_0 [[i1_11_uint]]
// VULKAN-NEXT: [[aumin11:%[0-9]+]] = OpAtomicUMin %uint [[ptr11]] %uint_5 %uint_0 [[i1_11_uint]]
// CHECK-NEXT: [[aumin11_int:%[0-9]+]] = OpBitcast %int [[aumin11]]
// CHECK-NEXT: OpStore %out_i1 [[aumin11_int]]
  // i1 should be cast to uint first.
  // AtomicUMin should be performed.
  // Result should be cast to int before being written to out_i1.
  InterlockedMin(g_tTex1du1[u1], i1, out_i1);



  /////////////////////////////////////////////////////////////////////////////
  /////    Test all Interlocked* functions on various resource types   ////////
  /////////////////////////////////////////////////////////////////////////////

// CHECK:      [[ptr12:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1_12:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT:       {{%[0-9]+}} = OpAtomicIAdd %int [[ptr12]] %uint_1 %uint_0 [[i1_12]]
// VULKAN-NEXT:       {{%[0-9]+}} = OpAtomicIAdd %int [[ptr12]] %uint_5 %uint_0 [[i1_12]]
  InterlockedAdd            (g_tTex1di1[u1], i1);

// CHECK:       [[ptr13:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT:  [[i1_13:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT: [[iadd13:%[0-9]+]] = OpAtomicIAdd %int [[ptr13]] %uint_1 %uint_0 [[i1_13]]
// VULKAN-NEXT: [[iadd13:%[0-9]+]] = OpAtomicIAdd %int [[ptr13]] %uint_5 %uint_0 [[i1_13]]
// CHECK-NEXT:                   OpStore %out_i1 [[iadd13]]
  InterlockedAdd            (g_tTex1di1[u1], i1, out_i1);

// CHECK:      [[ptr14:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1_14:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT:       {{%[0-9]+}} = OpAtomicAnd %int [[ptr14]] %uint_1 %uint_0 [[i1_14]]
// VULKAN-NEXT:       {{%[0-9]+}} = OpAtomicAnd %int [[ptr14]] %uint_5 %uint_0 [[i1_14]]
  InterlockedAnd            (g_tTex1di1[u1], i1);

// CHECK:      [[ptr15:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1_15:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT: [[and15:%[0-9]+]] = OpAtomicAnd %int [[ptr15]] %uint_1 %uint_0 [[i1_15]]
// VULKAN-NEXT: [[and15:%[0-9]+]] = OpAtomicAnd %int [[ptr15]] %uint_5 %uint_0 [[i1_15]]
// CHECK-NEXT:                  OpStore %out_i1 [[and15]]
  InterlockedAnd            (g_tTex1di1[u1], i1, out_i1);

// CHECK:      [[ptr16:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[u1_16:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT: {{%[0-9]+}} = OpAtomicUMax %uint [[ptr16]] %uint_1 %uint_0 [[u1_16]]
// VULKAN-NEXT: {{%[0-9]+}} = OpAtomicUMax %uint [[ptr16]] %uint_5 %uint_0 [[u1_16]]
  InterlockedMax(g_tTex1du1[u1], u1);

// CHECK:        [[u2_17:%[0-9]+]] = OpLoad %v2uint %u2
// CHECK-NEXT:   [[ptr17:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex2di1 [[u2_17]] %uint_0
// CHECK-NEXT:   [[i1_17:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT: [[asmax17:%[0-9]+]] = OpAtomicSMax %int [[ptr17]] %uint_1 %uint_0 [[i1_17]]
// VULKAN-NEXT: [[asmax17:%[0-9]+]] = OpAtomicSMax %int [[ptr17]] %uint_5 %uint_0 [[i1_17]]
// CHECK-NEXT:                    OpStore %out_i1 [[asmax17]]
  InterlockedMax(g_tTex2di1[u2], i1, out_i1);

// CHECK:      [[ptr18:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex2du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[u1_18:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT:       {{%[0-9]+}} = OpAtomicUMin %uint [[ptr18]] %uint_1 %uint_0 [[u1_18]]
// VULKAN-NEXT:       {{%[0-9]+}} = OpAtomicUMin %uint [[ptr18]] %uint_5 %uint_0 [[u1_18]]
  InterlockedMin(g_tTex2du1[u2], u1);

// CHECK:        [[u3_19:%[0-9]+]] = OpLoad %v3uint %u3
// CHECK-NEXT:   [[ptr19:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex3di1 [[u3_19]] %uint_0
// CHECK-NEXT:   [[i1_19:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT: [[asmin19:%[0-9]+]] = OpAtomicSMin %int [[ptr19]] %uint_1 %uint_0 [[i1_19]]
// VULKAN-NEXT: [[asmin19:%[0-9]+]] = OpAtomicSMin %int [[ptr19]] %uint_5 %uint_0 [[i1_19]]
// CHECK-NEXT:                    OpStore %out_i1 [[asmin19]]
  InterlockedMin(g_tTex3di1[u3], i1, out_i1);

// CHECK:      [[ptr20:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex3du1 {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[u1_20:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT:       {{%[0-9]+}} = OpAtomicOr %uint [[ptr20]] %uint_1 %uint_0 [[u1_20]]
// VULKAN-NEXT:       {{%[0-9]+}} = OpAtomicOr %uint [[ptr20]] %uint_5 %uint_0 [[u1_20]]
  InterlockedOr (g_tTex3du1[u3], u1);

// CHECK:      [[ptr21:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1a {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1_21:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT:  [[or21:%[0-9]+]] = OpAtomicOr %int [[ptr21]] %uint_1 %uint_0 [[i1_21]]
// VULKAN-NEXT:  [[or21:%[0-9]+]] = OpAtomicOr %int [[ptr21]] %uint_5 %uint_0 [[i1_21]]
// CHECK-NEXT:                  OpStore %out_i1 [[or21]]
  InterlockedOr (g_tTex1di1a[u2], i1, out_i1);

// CHECK:      [[ptr22:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1a {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[u1_22:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT:       {{%[0-9]+}} = OpAtomicXor %uint [[ptr22]] %uint_1 %uint_0 [[u1_22]]
// VULKAN-NEXT:       {{%[0-9]+}} = OpAtomicXor %uint [[ptr22]] %uint_5 %uint_0 [[u1_22]]
  InterlockedXor(g_tTex1du1a[u2], u1);

// CHECK:      [[ptr23:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tTex1di1a {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1_23:%[0-9]+]] = OpLoad %int %i1
// GLSL450-NEXT: [[xor23:%[0-9]+]] = OpAtomicXor %int [[ptr23]] %uint_1 %uint_0 [[i1_23]]
// VULKAN-NEXT: [[xor23:%[0-9]+]] = OpAtomicXor %int [[ptr23]] %uint_5 %uint_0 [[i1_23]]
// CHECK-NEXT:                  OpStore %out_i1 [[xor23]]
  InterlockedXor(g_tTex1di1a[u2], i1, out_i1);

// CHECK:       [[ptr24:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tTex1du1a {{%[0-9]+}} %uint_0
// CHECK-NEXT:  [[u1_24:%[0-9]+]] = OpLoad %uint %u1
// CHECK-NEXT: [[u1b_24:%[0-9]+]] = OpLoad %uint %u1b
// GLSL450-NEXT:        {{%[0-9]+}} = OpAtomicCompareExchange %uint [[ptr24]] %uint_1 %uint_0 %uint_0 [[u1b_24]] [[u1_24]]
// VULKAN-NEXT:        {{%[0-9]+}} = OpAtomicCompareExchange %uint [[ptr24]] %uint_5 %uint_0 %uint_0 [[u1b_24]] [[u1_24]]
  InterlockedCompareStore(g_tTex1du1a[u2], u1, u1b);

// CHECK:       [[ptr25:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_int %g_tBuffI {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[i1b_25:%[0-9]+]] = OpLoad %int %i1b
// CHECK-NEXT: [[i1c_25:%[0-9]+]] = OpLoad %int %i1c
// GLSL450-NEXT:  [[ace25:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr25]] %uint_1 %uint_0 %uint_0 [[i1c_25]] [[i1b_25]]
// VULKAN-NEXT:  [[ace25:%[0-9]+]] = OpAtomicCompareExchange %int [[ptr25]] %uint_5 %uint_0 %uint_0 [[i1c_25]] [[i1b_25]]
// CHECK-NEXT:                   OpStore %out_i1 [[ace25]]
  InterlockedCompareExchange(g_tBuffI[u1], i1b, i1c, out_i1);

// CHECK:      [[ptr26:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %g_tBuffU {{%[0-9]+}} %uint_0
// CHECK-NEXT: [[u1_26:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT:  [[ae26:%[0-9]+]] = OpAtomicExchange %uint [[ptr26]] %uint_1 %uint_0 [[u1_26]]
// VULKAN-NEXT:  [[ae26:%[0-9]+]] = OpAtomicExchange %uint [[ptr26]] %uint_5 %uint_0 [[u1_26]]
// CHECK-NEXT:                  OpStore %out_u1 [[ae26]]
  InterlockedExchange(g_tBuffU[u1], u1, out_u1);

// CHECK-NEXT:    [[u1:%[0-9]+]] = OpLoad %uint %u1
// CHECK-NEXT:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %g_tRWBuffU %int_0 [[u1]]
// CHECK-NEXT:    [[u1_0:%[0-9]+]] = OpLoad %uint %u1
// GLSL450-NEXT:   [[add:%[0-9]+]] = OpAtomicIAdd %uint [[ptr]] %uint_1 %uint_0 [[u1_0]]
// VULKAN-NEXT:   [[add:%[0-9]+]] = OpAtomicIAdd %uint [[ptr]] %uint_5 %uint_0 [[u1_0]]
// CHECK-NEXT:                  OpStore %out_u1 [[add]]
  InterlockedAdd(g_tRWBuffU[u1], u1, out_u1);
}

