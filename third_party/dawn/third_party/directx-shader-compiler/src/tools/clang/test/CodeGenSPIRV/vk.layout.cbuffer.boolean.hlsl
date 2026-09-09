// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_v3uint_uint_2 ArrayStride 16
// CHECK: OpMemberDecorate %FrameConstants 0 Offset 0
// CHECK: OpMemberDecorate %FrameConstants 1 Offset 4
// CHECK: OpMemberDecorate %FrameConstants 2 Offset 16
// CHECK: OpMemberDecorate %type_CONSTANTS 0 Offset 0
// CHECK: OpDecorate %type_CONSTANTS Block

// CHECK: [[v3uint0:%[0-9]+]] = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
// CHECK: [[v2uint0:%[0-9]+]] = OpConstantComposite %v2uint %uint_0 %uint_0

// CHECK: %T = OpTypeStruct %_arr_uint_uint_1
struct T {
  bool boolArray[1];
};

// CHECK: %FrameConstants = OpTypeStruct %uint %v3uint %_arr_v3uint_uint_2 %T
struct FrameConstants
{
  bool  boolScalar;
  bool3 boolVec;
  row_major bool2x3 boolMat;
  T t;
};

[[vk::binding(0, 0)]]
cbuffer CONSTANTS
{
  FrameConstants frameConstants;
};

// These are the types that hold SPIR-V booleans, rather than Uints.
// CHECK:              %T_0 = OpTypeStruct %_arr_bool_uint_1
// CHECK: %FrameConstants_0 = OpTypeStruct %bool %v3bool %_arr_v3bool_uint_2 %T_0

float4 main(in float4 texcoords : TEXCOORD0) : SV_TARGET
{
// CHECK:      [[FrameConstants:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintVecPtr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3uint [[FrameConstants]] %int_1
// CHECK-NEXT:        [[uintVec:%[0-9]+]] = OpLoad %v3uint [[uintVecPtr]]
// CHECK-NEXT:        [[boolVec:%[0-9]+]] = OpINotEqual %v3bool [[uintVec]] [[v3uint0]]
// CHECK-NEXT:                           OpStore %a [[boolVec]]
    bool3   a = frameConstants.boolVec;

// CHECK:      [[FrameConstants_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:        [[uintPtr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[FrameConstants_0]] %int_1 %uint_0
// CHECK-NEXT:           [[uint:%[0-9]+]] = OpLoad %uint [[uintPtr]]
// CHECK-NEXT:           [[bool:%[0-9]+]] = OpINotEqual %bool [[uint]] %uint_0
// CHECK-NEXT:                           OpStore %b [[bool]]
    bool    b = frameConstants.boolVec[0];

// CHECK:      [[FrameConstants_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:        [[uintPtr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[FrameConstants_1]] %int_0
// CHECK-NEXT:           [[uint_0:%[0-9]+]] = OpLoad %uint [[uintPtr_0]]
// CHECK-NEXT:           [[bool_0:%[0-9]+]] = OpINotEqual %bool [[uint_0]] %uint_0
// CHECK-NEXT:                           OpStore %c [[bool_0]]
    bool    c = frameConstants.boolScalar;

// CHECK:      [[FrameConstants_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintMatPtr:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v3uint_uint_2 [[FrameConstants_2]] %int_2
// CHECK-NEXT:        [[uintMat:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 [[uintMatPtr]]
// CHECK-NEXT:       [[uintVec1:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 0
// CHECK-NEXT:       [[boolVec1:%[0-9]+]] = OpINotEqual %v3bool [[uintVec1]] [[v3uint0]]
// CHECK-NEXT:       [[uintVec2:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 1
// CHECK-NEXT:       [[boolVec2:%[0-9]+]] = OpINotEqual %v3bool [[uintVec2]] [[v3uint0]]
// CHECK-NEXT:        [[boolMat:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[boolVec1]] [[boolVec2]]
// CHECK-NEXT:                           OpStore %d [[boolMat]]
    bool2x3 d = frameConstants.boolMat;

// CHECK:      [[FrameConstants_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintVecPtr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3uint [[FrameConstants_3]] %int_2 %uint_0
// CHECK-NEXT:        [[uintVec_0:%[0-9]+]] = OpLoad %v3uint [[uintVecPtr_0]]
// CHECK-NEXT:        [[boolVec_0:%[0-9]+]] = OpINotEqual %v3bool [[uintVec_0]] [[v3uint0]]
// CHECK-NEXT:                           OpStore %e [[boolVec_0]]
    bool3   e = frameConstants.boolMat[0];

// CHECK:      [[FrameConstants_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:        [[uintPtr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[FrameConstants_4]] %int_2 %uint_1 %uint_2
// CHECK-NEXT:           [[uint_1:%[0-9]+]] = OpLoad %uint [[uintPtr_1]]
// CHECK-NEXT:           [[bool_1:%[0-9]+]] = OpINotEqual %bool [[uint_1]] %uint_0
// CHECK-NEXT:                           OpStore %f [[bool_1]]
    bool    f = frameConstants.boolMat[1][2];

// Swizzle Vector: out of order
// CHECK:      [[FrameConstants_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintVecPtr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3uint [[FrameConstants_5]] %int_1
// CHECK-NEXT:        [[uintVec_1:%[0-9]+]] = OpLoad %v3uint [[uintVecPtr_1]]
// CHECK-NEXT:        [[boolVec_1:%[0-9]+]] = OpINotEqual %v3bool [[uintVec_1]] [[v3uint0]]
// CHECK-NEXT:        [[swizzle:%[0-9]+]] = OpVectorShuffle %v2bool [[boolVec_1]] [[boolVec_1]] 1 0
// CHECK-NEXT:                           OpStore %g [[swizzle]]
    bool2   g = frameConstants.boolVec.yx;

// Swizzle Vector: one element only.
// CHECK:      [[FrameConstants_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintVecPtr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3uint [[FrameConstants_6]] %int_1
// CHECK-NEXT:        [[uintPtr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[uintVecPtr_2]] %int_0
// CHECK-NEXT:           [[uint_2:%[0-9]+]] = OpLoad %uint [[uintPtr_2]]
// CHECK-NEXT:           [[bool_2:%[0-9]+]] = OpINotEqual %bool [[uint_2]] %uint_0
// CHECK-NEXT:                           OpStore %h [[bool_2]]
    bool    h = frameConstants.boolVec.x;

// Swizzle Vector: original indeces.
// CHECK:      [[FrameConstants_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintVecPtr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3uint [[FrameConstants_7]] %int_1
// CHECK-NEXT:        [[uintVec_2:%[0-9]+]] = OpLoad %v3uint [[uintVecPtr_3]]
// CHECK-NEXT:        [[boolVec_2:%[0-9]+]] = OpINotEqual %v3bool [[uintVec_2]] [[v3uint0]]
// CHECK-NEXT:                           OpStore %i [[boolVec_2]]
    bool3   i = frameConstants.boolVec.xyz;

// Swizzle Vector: on temporary value (rvalue)
// CHECK:       [[uintVec1_0:%[0-9]+]] = OpLoad %v3uint {{%[0-9]+}}
// CHECK-NEXT:  [[boolVec1_0:%[0-9]+]] = OpINotEqual %v3bool [[uintVec1_0]] [[v3uint0]]
// CHECK:       [[uintVec2_0:%[0-9]+]] = OpLoad %v3uint {{%[0-9]+}}
// CHECK-NEXT:  [[boolVec2_0:%[0-9]+]] = OpINotEqual %v3bool [[uintVec2_0]] [[v3uint0]]
// CHECK-NEXT: [[temporary:%[0-9]+]] = OpLogicalAnd %v3bool [[boolVec1_0]] [[boolVec2_0]]
// CHECK-NEXT:      [[bool_3:%[0-9]+]] = OpCompositeExtract %bool [[temporary]] 0
// CHECK-NEXT:                      OpStore %j [[bool_3]]
    bool    j = (frameConstants.boolVec && frameConstants.boolVec).x;

// CHECK:      [[FrameConstants_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:     [[uintMatPtr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v3uint_uint_2 [[FrameConstants_8]] %int_2
// CHECK-NEXT:        [[uintPtr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[uintMatPtr_0]] %int_1 %int_2
// CHECK-NEXT:          [[uint0:%[0-9]+]] = OpLoad %uint [[uintPtr_3]]
// CHECK-NEXT:        [[uintPtr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[uintMatPtr_0]] %int_0 %int_1
// CHECK-NEXT:          [[uint1:%[0-9]+]] = OpLoad %uint [[uintPtr_4]]
// CHECK-NEXT:        [[uintVec_3:%[0-9]+]] = OpCompositeConstruct %v2uint [[uint0]] [[uint1]]
// CHECK-NEXT:        [[boolVec_3:%[0-9]+]] = OpINotEqual %v2bool [[uintVec_3]] [[v2uint0]]
// CHECK-NEXT:                           OpStore %k [[boolVec_3]]
    bool2   k = frameConstants.boolMat._m12_m01;

// CHECK:      [[FrameConstants_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:            [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_uint_uint_1 [[FrameConstants_9]] %int_3 %int_0
// CHECK-NEXT:            [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[base]] %int_2
// CHECK-NEXT:           [[uint_3:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK-NEXT:           [[bool_4:%[0-9]+]] = OpINotEqual %bool [[uint_3]] %uint_0
// CHECK-NEXT:                           OpStore %l [[bool_4]]
    bool    l = frameConstants.t.boolArray[2];

// CHECK:           [[FrameConstantsPtr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FrameConstants %CONSTANTS %int_0
// CHECK-NEXT:         [[FrameConstants_10:%[0-9]+]] = OpLoad %FrameConstants [[FrameConstantsPtr]]
// CHECK-NEXT:              [[fc_0_uint:%[0-9]+]] = OpCompositeExtract %uint [[FrameConstants_10]] 0
// CHECK-NEXT:              [[fc_0_bool:%[0-9]+]] = OpINotEqual %bool [[fc_0_uint]] %uint_0
// CHECK-NEXT:             [[fc_1_uint3:%[0-9]+]] = OpCompositeExtract %v3uint [[FrameConstants_10]] 1
// CHECK-NEXT:             [[fc_1_bool3:%[0-9]+]] = OpINotEqual %v3bool [[fc_1_uint3]] [[v3uint0]]
// CHECK-NEXT:           [[fc_2_uintMat:%[0-9]+]] = OpCompositeExtract %_arr_v3uint_uint_2 [[FrameConstants_10]] 2
// CHECK-NEXT: [[fc_2_uintMat_row0_uint:%[0-9]+]] = OpCompositeExtract %v3uint [[fc_2_uintMat]] 0
// CHECK-NEXT: [[fc_2_uintMat_row0_bool:%[0-9]+]] = OpINotEqual %v3bool [[fc_2_uintMat_row0_uint]] [[v3uint0]]
// CHECK-NEXT: [[fc_2_uintMat_row1_uint:%[0-9]+]] = OpCompositeExtract %v3uint [[fc_2_uintMat]] 1
// CHECK-NEXT: [[fc_2_uintMat_row1_bool:%[0-9]+]] = OpINotEqual %v3bool [[fc_2_uintMat_row1_uint]] [[v3uint0]]
// CHECK-NEXT:           [[fc_2_boolMat:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[fc_2_uintMat_row0_bool]] [[fc_2_uintMat_row1_bool]]
// CHECK-NEXT:                 [[fc_3_T:%[0-9]+]] = OpCompositeExtract %T [[FrameConstants_10]] 3
// CHECK-NEXT:      [[fc_3_T_0_uint_arr:%[0-9]+]] = OpCompositeExtract %_arr_uint_uint_1 [[fc_3_T]] 0
// CHECK-NEXT:        [[fc_3_T_1_uint:%[0-9]+]] = OpCompositeExtract %uint [[fc_3_T_0_uint_arr]] 0
// CHECK-NEXT:        [[fc_3_T_1_bool:%[0-9]+]] = OpINotEqual %bool [[fc_3_T_1_uint]] %uint_0
// CHECK-NEXT:      [[fc_3_T_0_bool_arr:%[0-9]+]] = OpCompositeConstruct %_arr_bool_uint_1 [[fc_3_T_1_bool]]
// CHECK-NEXT:            [[fc_3_T_bool:%[0-9]+]] = OpCompositeConstruct %T_0 [[fc_3_T_0_bool_arr]]
// CHECK-NEXT:                     [[fc:%[0-9]+]] = OpCompositeConstruct %FrameConstants_0 [[fc_0_bool]] [[fc_1_bool3]] [[fc_2_boolMat]] [[fc_3_T_bool]]
// CHECK-NEXT:                                   OpStore %fc [[fc]]
    FrameConstants fc = frameConstants;

    return (1.0).xxxx;
}
