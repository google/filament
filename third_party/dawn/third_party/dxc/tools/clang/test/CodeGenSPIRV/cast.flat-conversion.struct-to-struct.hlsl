// RUN: %dxc -T cs_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// Processing FlatConversion when source and destination
// are both structures with identical members.

struct FirstStruct {
  float3 anArray[4];
  float2x3 mats[1];
  int2 ints[3];
};

struct SecondStruct {
  float3 anArray[4];
  float2x3 mats[1];
  int2 ints[3];
};

RWStructuredBuffer<FirstStruct> rwBuf : register(u0);
[ numthreads ( 16 , 16 , 1 ) ]
void main() {
  SecondStruct values;
  FirstStruct v;

// Yes, this is a FlatConversion!
// CHECK:      [[values:%[0-9]+]] = OpLoad %SecondStruct %values
// CHECK-NEXT:     [[v0:%[0-9]+]] = OpCompositeExtract %_arr_v3float_uint_4_0 [[values]] 0
// CHECK-NEXT:     [[v1:%[0-9]+]] = OpCompositeExtract %_arr_mat2v3float_uint_1_0 [[values]] 1
// CHECK-NEXT:     [[v2:%[0-9]+]] = OpCompositeExtract %_arr_v2int_uint_3_0 [[values]] 2
// CHECK-NEXT:      [[v:%[0-9]+]] = OpCompositeConstruct %FirstStruct_0 [[v0]] [[v1]] [[v2]]
// CHECK-NEXT:                   OpStore %v [[v]]
  v = values;

// CHECK-NEXT: [[values_0:%[0-9]+]] = OpLoad %SecondStruct %values
// CHECK-NEXT:     [[v0_0:%[0-9]+]] = OpCompositeExtract %_arr_v3float_uint_4_0 [[values_0]] 0
// CHECK-NEXT:     [[v1_0:%[0-9]+]] = OpCompositeExtract %_arr_mat2v3float_uint_1_0 [[values_0]] 1
// CHECK-NEXT:     [[v2_0:%[0-9]+]] = OpCompositeExtract %_arr_v2int_uint_3_0 [[values_0]] 2
// CHECK-NEXT:    [[v_0:%[0-9]+]] = OpCompositeConstruct %FirstStruct_0 [[v0_0]] [[v1_0]] [[v2_0]]
// CHECK-NEXT: [[rwBuf_ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_FirstStruct %rwBuf %int_0 %uint_0
// CHECK-NEXT:   [[anArray:%[0-9]+]] = OpCompositeExtract %_arr_v3float_uint_4_0 [[v_0]] 0
// CHECK-NEXT:  [[anArray1:%[0-9]+]] = OpCompositeExtract %v3float [[anArray]] 0
// CHECK-NEXT:  [[anArray2:%[0-9]+]] = OpCompositeExtract %v3float [[anArray]] 1
// CHECK-NEXT:  [[anArray3:%[0-9]+]] = OpCompositeExtract %v3float [[anArray]] 2
// CHECK-NEXT:  [[anArray4:%[0-9]+]] = OpCompositeExtract %v3float [[anArray]] 3
// CHECK-NEXT:      [[res1:%[0-9]+]] = OpCompositeConstruct %_arr_v3float_uint_4 [[anArray1]] [[anArray2]] [[anArray3]] [[anArray4]]

// CHECK-NEXT:      [[mats:%[0-9]+]] = OpCompositeExtract %_arr_mat2v3float_uint_1_0 [[v_0]] 1
// CHECK-NEXT:       [[mat:%[0-9]+]] = OpCompositeExtract %mat2v3float [[mats]] 0
// CHECK-NEXT:      [[res2:%[0-9]+]] = OpCompositeConstruct %_arr_mat2v3float_uint_1 [[mat]]

// CHECK-NEXT:      [[ints:%[0-9]+]] = OpCompositeExtract %_arr_v2int_uint_3_0 [[v_0]] 2
// CHECK-NEXT:     [[ints1:%[0-9]+]] = OpCompositeExtract %v2int [[ints]] 0
// CHECK-NEXT:     [[ints2:%[0-9]+]] = OpCompositeExtract %v2int [[ints]] 1
// CHECK-NEXT:     [[ints3:%[0-9]+]] = OpCompositeExtract %v2int [[ints]] 2
// CHECK-NEXT:      [[res3:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[ints1]] [[ints2]] [[ints3]]

// CHECK-NEXT:    [[result:%[0-9]+]] = OpCompositeConstruct %FirstStruct [[res1]] [[res2]] [[res3]]
// CHECK-NEXT:                      OpStore [[rwBuf_ptr]] [[result]]
  rwBuf[0] = values;
}
