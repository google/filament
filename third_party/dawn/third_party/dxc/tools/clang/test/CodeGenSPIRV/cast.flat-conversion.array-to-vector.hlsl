// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

typedef int A[2];

ConsumeStructuredBuffer<A> intarr_consume;

A getA() {
  A a = intarr_consume.Consume();
  return a;
}

typedef float B[2];

ConsumeStructuredBuffer<B> floatarr_consume;

B getB() {
  B b = floatarr_consume.Consume();
  return b;
}

typedef bool C[2];

ConsumeStructuredBuffer<C> boolarr_consume;

C getC() {
  C c = boolarr_consume.Consume();
  return c;
}

void main() {
  // CHECK:      [[call_0:%[0-9]+]] = OpFunctionCall %_arr_int_uint_2_0 %getA
  // CHECK-NEXT:  [[i_1:%[0-9]+]] = OpCompositeExtract %int [[call_0]] 0
  // CHECK-NEXT:  [[i_0_1:%[0-9]+]] = OpCompositeExtract %int [[call_0]] 1
  // CHECK-NEXT:   [[vi_0:%[0-9]+]] = OpCompositeConstruct %v2int [[i_1]] [[i_0_1]]
  // CHECK-NEXT:                   OpStore %intarray_to_intvec [[vi_0]]
  int2 intarray_to_intvec = (int2)getA();

  // CHECK:      [[call_1:%[0-9]+]] = OpFunctionCall %_arr_float_uint_2_0 %getB
  // CHECK-NEXT:  [[f_1_0:%[0-9]+]] = OpCompositeExtract %float [[call_1]] 0
  // CHECK-NEXT:  [[f_1_1:%[0-9]+]] = OpCompositeExtract %float [[call_1]] 1
  // CHECK-NEXT:   [[vf_1:%[0-9]+]] = OpCompositeConstruct %v2float [[f_1_0]] [[f_1_1]]
  // CHECK-NEXT:                   OpStore %floatarray_to_floatvec [[vf_1]]
  float2 floatarray_to_floatvec = (float2)getB();

  // CHECK:      [[call_2:%[0-9]+]] = OpFunctionCall %_arr_bool_uint_2 %getC
  // CHECK-NEXT:  [[b_2_0:%[0-9]+]] = OpCompositeExtract %bool [[call_2]] 0
  // CHECK-NEXT:  [[b_2_1:%[0-9]+]] = OpCompositeExtract %bool [[call_2]] 1
  // CHECK-NEXT:   [[vb_2:%[0-9]+]] = OpCompositeConstruct %v2bool [[b_2_0]] [[b_2_1]]
  // CHECK-NEXT:                   OpStore %boolarray_to_boolvec [[vb_2]]
  bool2 boolarray_to_boolvec = (bool2)getC();

  // CHECK:      [[call_3:%[0-9]+]] = OpFunctionCall %_arr_float_uint_2_0 %getB
  // CHECK-NEXT:  [[f_3_0:%[0-9]+]] = OpCompositeExtract %float [[call_3]] 0
  // CHECK-NEXT:  [[f_3_1:%[0-9]+]] = OpCompositeExtract %float [[call_3]] 1
  // CHECK-NEXT:  [[i_3_0:%[0-9]+]] = OpConvertFToS %int [[f_3_0]]
  // CHECK-NEXT:  [[i_3_1:%[0-9]+]] = OpConvertFToS %int [[f_3_1]]
  // CHECK-NEXT:   [[vi_3:%[0-9]+]] = OpCompositeConstruct %v2int [[i_3_0]] [[i_3_1]]
  // CHECK-NEXT:                   OpStore %floatarray_to_intvec [[vi_3]]
  int2 floatarray_to_intvec = (int2)getB();

  // CHECK:      [[call_4:%[0-9]+]] = OpFunctionCall %_arr_bool_uint_2 %getC
  // CHECK-NEXT:  [[b_4_0:%[0-9]+]] = OpCompositeExtract %bool [[call_4]] 0
  // CHECK-NEXT:  [[b_4_1:%[0-9]+]] = OpCompositeExtract %bool [[call_4]] 1
  // CHECK-NEXT:  [[i_4_0:%[0-9]+]] = OpSelect %int [[b_4_0]] %int_1 %int_0
  // CHECK-NEXT:  [[i_4_1:%[0-9]+]] = OpSelect %int [[b_4_1]] %int_1 %int_0
  // CHECK-NEXT:   [[vi_4:%[0-9]+]] = OpCompositeConstruct %v2int [[i_4_0]] [[i_4_1]]
  // CHECK-NEXT:                   OpStore %boolarray_to_intvec [[vi_4]]
  int2 boolarray_to_intvec = (int2)getC();

  // CHECK:      [[call_5:%[0-9]+]] = OpFunctionCall %_arr_int_uint_2_0 %getA
  // CHECK-NEXT:  [[i_5_0:%[0-9]+]] = OpCompositeExtract %int [[call_5]] 0
  // CHECK-NEXT:  [[i_5_1:%[0-9]+]] = OpCompositeExtract %int [[call_5]] 1
  // CHECK-NEXT:  [[f_5_0:%[0-9]+]] = OpConvertSToF %float [[i_5_0]]
  // CHECK-NEXT:  [[f_5_1:%[0-9]+]] = OpConvertSToF %float [[i_5_1]]
  // CHECK-NEXT:   [[vf_5:%[0-9]+]] = OpCompositeConstruct %v2float [[f_5_0]] [[f_5_1]]
  // CHECK-NEXT:                   OpStore %intarray_to_floatvec [[vf_5]]
  float2 intarray_to_floatvec = (float2)getA();

  // CHECK:      [[call_6:%[0-9]+]] = OpFunctionCall %_arr_bool_uint_2 %getC
  // CHECK-NEXT:  [[b_6_0:%[0-9]+]] = OpCompositeExtract %bool [[call_6]] 0
  // CHECK-NEXT:  [[b_6_1:%[0-9]+]] = OpCompositeExtract %bool [[call_6]] 1
  // CHECK-NEXT:  [[f_6_0:%[0-9]+]] = OpSelect %float [[b_6_0]] %float_1 %float_0
  // CHECK-NEXT:  [[f_6_1:%[0-9]+]] = OpSelect %float [[b_6_1]] %float_1 %float_0
  // CHECK-NEXT:   [[vf_6:%[0-9]+]] = OpCompositeConstruct %v2float [[f_6_0]] [[f_6_1]]
  // CHECK-NEXT:                   OpStore %boolarray_to_floatvec [[vf_6]]
  float2 boolarray_to_floatvec = (float2)getC();

  // CHECK:      [[call_7:%[0-9]+]] = OpFunctionCall %_arr_int_uint_2_0 %getA
  // CHECK-NEXT:  [[i_7_0:%[0-9]+]] = OpCompositeExtract %int [[call_7]] 0
  // CHECK-NEXT:  [[i_7_1:%[0-9]+]] = OpCompositeExtract %int [[call_7]] 1
  // CHECK-NEXT:  [[b_7_0:%[0-9]+]] = OpINotEqual %bool [[i_7_0]] %int_0
  // CHECK-NEXT:  [[b_7_1:%[0-9]+]] = OpINotEqual %bool [[i_7_1]] %int_0
  // CHECK-NEXT:   [[vb_7:%[0-9]+]] = OpCompositeConstruct %v2bool [[b_7_0]] [[b_7_1]]
  // CHECK-NEXT:                   OpStore %intarray_to_boolvec [[vb_7]]
  bool2 intarray_to_boolvec = (bool2)getA();

  // CHECK:      [[call_8:%[0-9]+]] = OpFunctionCall %_arr_float_uint_2_0 %getB
  // CHECK-NEXT:  [[f_8_0:%[0-9]+]] = OpCompositeExtract %float [[call_8]] 0
  // CHECK-NEXT:  [[f_8_1:%[0-9]+]] = OpCompositeExtract %float [[call_8]] 1
  // CHECK-NEXT:  [[b_8_0:%[0-9]+]] = OpFOrdNotEqual %bool [[f_8_0]] %float_0
  // CHECK-NEXT:  [[b_8_1:%[0-9]+]] = OpFOrdNotEqual %bool [[f_8_1]] %float_0
  // CHECK-NEXT:   [[vb_8:%[0-9]+]] = OpCompositeConstruct %v2bool [[b_8_0]] [[b_8_1]]
  // CHECK-NEXT:                   OpStore %floatarray_to_boolvec [[vb_8]]
  bool2 floatarray_to_boolvec = (bool2)getB();
}
