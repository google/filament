// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct VSOutput {
  float4   sv_pos     : SV_POSITION;
  uint3    normal     : NORMAL;
  int2     tex_coord  : TEXCOORD;
  bool     mybool[2]  : MYBOOL;
  int      arr[5]     : MYARRAY;
  float2x3 mat2x3     : MYMATRIX;
  int2x3   intmat     : MYINTMATRIX;
  bool2x3  boolmat    : MYBOOLMATRIX;
};


// CHECK: [[nullVSOutput:%[0-9]+]] = OpConstantNull %VSOutput


void main() {
  int x = 3;

// CHECK: OpStore %output1 [[nullVSOutput]]
  VSOutput output1 = (VSOutput)0;
// CHECK: OpStore %output2 [[nullVSOutput]]
  VSOutput output2 = (VSOutput)0.0;
// CHECK: OpStore %output3 [[nullVSOutput]]
  VSOutput output3 = (VSOutput)false;

// CHECK:                [[f1:%[0-9]+]] = OpConvertSToF %float %int_1
// CHECK-NEXT:         [[v4f1:%[0-9]+]] = OpCompositeConstruct %v4float [[f1]] [[f1]] [[f1]] [[f1]]
// CHECK-NEXT:           [[u1:%[0-9]+]] = OpBitcast %uint %int_1
// CHECK-NEXT:         [[v3u1:%[0-9]+]] = OpCompositeConstruct %v3uint [[u1]] [[u1]] [[u1]]
// CHECK-NEXT:         [[v2i1:%[0-9]+]] = OpCompositeConstruct %v2int %int_1 %int_1
// CHECK-NEXT:        [[bool1:%[0-9]+]] = OpINotEqual %bool %int_1 %int_0
// CHECK-NEXT:    [[arr2bool1:%[0-9]+]] = OpCompositeConstruct %_arr_bool_uint_2 [[bool1]] [[bool1]]
// CHECK-NEXT:       [[arr5i1:%[0-9]+]] = OpCompositeConstruct %_arr_int_uint_5 %int_1 %int_1 %int_1 %int_1 %int_1
// CHECK-NEXT:         [[f1_1:%[0-9]+]] = OpConvertSToF %float %int_1
// CHECK-NEXT:         [[col3:%[0-9]+]] = OpCompositeConstruct %v3float [[f1_1]] [[f1_1]] [[f1_1]]
// CHECK-NEXT:    [[matFloat1:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[col3]] [[col3]]
// CHECK-NEXT:         [[v3i1:%[0-9]+]] = OpCompositeConstruct %v3int %int_1 %int_1 %int_1
// CHECK-NEXT:       [[intmat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[v3i1]] [[v3i1]]
// CHECK-NEXT:         [[true:%[0-9]+]] = OpINotEqual %bool %int_1 %int_0
// CHECK-NEXT:      [[boolvec:%[0-9]+]] = OpCompositeConstruct %v3bool [[true]] [[true]] [[true]]
// CHECK-NEXT:      [[boolmat:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[boolvec]] [[boolvec]]
// CHECK-NEXT: [[flatConvert1:%[0-9]+]] = OpCompositeConstruct %VSOutput [[v4f1]] [[v3u1]] [[v2i1]] [[arr2bool1]] [[arr5i1]] [[matFloat1]] [[intmat]] [[boolmat]]
// CHECK-NEXT:                         OpStore %output4 [[flatConvert1]]
  VSOutput output4 = (VSOutput)1;

// CHECK:                [[x:%[0-9]+]] = OpLoad %int %x
// CHECK-NEXT:       [[floatX:%[0-9]+]] = OpConvertSToF %float [[x]]
// CHECK-NEXT:         [[v4fX:%[0-9]+]] = OpCompositeConstruct %v4float [[floatX]] [[floatX]] [[floatX]] [[floatX]]
// CHECK-NEXT:        [[uintX:%[0-9]+]] = OpBitcast %uint [[x]]
// CHECK-NEXT:         [[v3uX:%[0-9]+]] = OpCompositeConstruct %v3uint [[uintX]] [[uintX]] [[uintX]]
// CHECK-NEXT:         [[v2iX:%[0-9]+]] = OpCompositeConstruct %v2int [[x]] [[x]]
// CHECK-NEXT:        [[boolX:%[0-9]+]] = OpINotEqual %bool [[x]] %int_0
// CHECK-NEXT:    [[arr2boolX:%[0-9]+]] = OpCompositeConstruct %_arr_bool_uint_2 [[boolX]] [[boolX]]
// CHECK-NEXT:       [[arr5iX:%[0-9]+]] = OpCompositeConstruct %_arr_int_uint_5 [[x]] [[x]] [[x]] [[x]] [[x]]
// CHECK-NEXT:      [[floatX2:%[0-9]+]] = OpConvertSToF %float [[x]]
// CHECK-NEXT:         [[v3fX:%[0-9]+]] = OpCompositeConstruct %v3float [[floatX2]] [[floatX2]] [[floatX2]]
// CHECK-NEXT:    [[matFloatX:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[v3fX]] [[v3fX]]
// CHECK-NEXT:       [[intvec:%[0-9]+]] = OpCompositeConstruct %v3int [[x]] [[x]] [[x]]
// CHECK-NEXT:       [[intmat_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[intvec]] [[intvec]]
// CHECK-NEXT:        [[boolx:%[0-9]+]] = OpINotEqual %bool [[x]] %int_0
// CHECK-NEXT:      [[boolvec_0:%[0-9]+]] = OpCompositeConstruct %v3bool [[boolx]] [[boolx]] [[boolx]]
// CHECK-NEXT:      [[boolmat_0:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[boolvec_0]] [[boolvec_0]]
// CHECK-NEXT: [[flatConvert2:%[0-9]+]] = OpCompositeConstruct %VSOutput [[v4fX]] [[v3uX]] [[v2iX]] [[arr2boolX]] [[arr5iX]] [[matFloatX]] [[intmat_0]] [[boolmat_0]]
// CHECK-NEXT:                         OpStore %output5 [[flatConvert2]]
  VSOutput output5 = (VSOutput)x;

// CHECK:            [[v4f1_5:%[0-9]+]] = OpCompositeConstruct %v4float %float_1_5 %float_1_5 %float_1_5 %float_1_5
// CHECK-NEXT:         [[u1_5:%[0-9]+]] = OpConvertFToU %uint %float_1_5
// CHECK-NEXT:       [[v3u1_5:%[0-9]+]] = OpCompositeConstruct %v3uint [[u1_5]] [[u1_5]] [[u1_5]]
// CHECK-NEXT:         [[i1_5:%[0-9]+]] = OpConvertFToS %int %float_1_5
// CHECK-NEXT:       [[v2i1_5:%[0-9]+]] = OpCompositeConstruct %v2int [[i1_5]] [[i1_5]]
// CHECK-NEXT:      [[bool1_5:%[0-9]+]] = OpFOrdNotEqual %bool %float_1_5 %float_0
// CHECK-NEXT: [[arr2bool_1_5:%[0-9]+]] = OpCompositeConstruct %_arr_bool_uint_2 [[bool1_5]] [[bool1_5]]
// CHECK-NEXT:         [[i1_5_0:%[0-9]+]] = OpConvertFToS %int %float_1_5
// CHECK-NEXT:     [[arr5i1_5:%[0-9]+]] = OpCompositeConstruct %_arr_int_uint_5 [[i1_5_0]] [[i1_5_0]] [[i1_5_0]] [[i1_5_0]] [[i1_5_0]]
// CHECK-NEXT:      [[v3f_1_5:%[0-9]+]] = OpCompositeConstruct %v3float %float_1_5 %float_1_5 %float_1_5
// CHECK-NEXT: [[matFloat_1_5:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[v3f_1_5]] [[v3f_1_5]]
// CHECK-NEXT:      [[int_1_5:%[0-9]+]] = OpConvertFToS %int %float_1_5
// CHECK-NEXT:       [[intvec_0:%[0-9]+]] = OpCompositeConstruct %v3int [[int_1_5]] [[int_1_5]] [[int_1_5]]
// CHECK-NEXT:       [[intmat_1:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[intvec_0]] [[intvec_0]]
// CHECK-NEXT:     [[bool_1_5:%[0-9]+]] = OpFOrdNotEqual %bool %float_1_5 %float_0
// CHECK-NEXT:      [[boolvec_1:%[0-9]+]] = OpCompositeConstruct %v3bool [[bool_1_5]] [[bool_1_5]] [[bool_1_5]]
// CHECK-NEXT:      [[boolmat_1:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[boolvec_1]] [[boolvec_1]]
// CHECK-NEXT:              {{%[0-9]+}} = OpCompositeConstruct %VSOutput [[v4f1_5]] [[v3u1_5]] [[v2i1_5]] [[arr2bool_1_5]] [[arr5i1_5]] [[matFloat_1_5]] [[intmat_1]] [[boolmat_1]]
  VSOutput output6 = (VSOutput)1.5;

// CHECK:      [[float_true:%[0-9]+]] = OpSelect %float %true %float_1 %float_0
// CHECK-NEXT:   [[v4f_true:%[0-9]+]] = OpCompositeConstruct %v4float [[float_true]] [[float_true]] [[float_true]] [[float_true]]
// CHECK-NEXT:  [[uint_true:%[0-9]+]] = OpSelect %uint %true %uint_1 %uint_0
// CHECK-NEXT:   [[v3u_true:%[0-9]+]] = OpCompositeConstruct %v3uint [[uint_true]] [[uint_true]] [[uint_true]]
// CHECK-NEXT:   [[int_true:%[0-9]+]] = OpSelect %int %true %int_1 %int_0
// CHECK-NEXT:   [[v2i_true:%[0-9]+]] = OpCompositeConstruct %v2int [[int_true]] [[int_true]]
// CHECK-NEXT:  [[arr2_true:%[0-9]+]] = OpCompositeConstruct %_arr_bool_uint_2 %true %true
// CHECK-NEXT:   [[int_true_0:%[0-9]+]] = OpSelect %int %true %int_1 %int_0
// CHECK-NEXT: [[arr5i_true:%[0-9]+]] = OpCompositeConstruct %_arr_int_uint_5 [[int_true_0]] [[int_true_0]] [[int_true_0]] [[int_true_0]] [[int_true_0]]
// CHECK-NEXT: [[float_true_0:%[0-9]+]] = OpSelect %float %true %float_1 %float_0
// CHECK-NEXT:   [[v3f_true:%[0-9]+]] = OpCompositeConstruct %v3float [[float_true_0]] [[float_true_0]] [[float_true_0]]
// CHECK-NEXT:[[mat2v3_true:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[v3f_true]] [[v3f_true]]
// CHECK-NEXT:   [[true_int:%[0-9]+]] = OpSelect %int %true %int_1 %int_0
// CHECK-NEXT:     [[intvec_1:%[0-9]+]] = OpCompositeConstruct %v3int [[true_int]] [[true_int]] [[true_int]]
// CHECK-NEXT:     [[intmat_2:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[intvec_1]] [[intvec_1]]
// CHECK-NEXT:    [[boolvec_2:%[0-9]+]] = OpCompositeConstruct %v3bool %true %true %true
// CHECK-NEXT:    [[boolmat_2:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[boolvec_2]] [[boolvec_2]]
// CHECK-NEXT:            {{%[0-9]+}} = OpCompositeConstruct %VSOutput [[v4f_true]] [[v3u_true]] [[v2i_true]] [[arr2_true]] [[arr5i_true]] [[mat2v3_true]] [[intmat_2]] [[boolmat_2]]
  VSOutput output7 = (VSOutput)true;

}
