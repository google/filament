// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct struct_with_bool {
  bool2 elem_v2bool;
  int2 elem_v2int;
  float2 elem_v2float;
  bool elem_bool;
  int elem_int;
  float elem_float;
};

ConsumeStructuredBuffer<bool2> consume_v2bool;
ConsumeStructuredBuffer<float2> consume_v2float;
ConsumeStructuredBuffer<int2> consume_v2int;
ConsumeStructuredBuffer<struct_with_bool> consume_struct_with_bool;

ConsumeStructuredBuffer<bool> consume_bool;
ConsumeStructuredBuffer<float> consume_float;
ConsumeStructuredBuffer<int> consume_int;

AppendStructuredBuffer<bool2> append_v2bool;
AppendStructuredBuffer<float2> append_v2float;
AppendStructuredBuffer<int2> append_v2int;
AppendStructuredBuffer<struct_with_bool> append_struct_with_bool;

AppendStructuredBuffer<bool> append_bool;
AppendStructuredBuffer<float> append_float;
AppendStructuredBuffer<int> append_int;

RWStructuredBuffer<bool> rw_bool;
RWStructuredBuffer<bool2> rw_v2bool;

ConsumeStructuredBuffer<float2x2> consume_float2x2;
AppendStructuredBuffer<float4> append_v4float;

[numthreads(1,1,1)]
void main() {
// CHECK:       [[p_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %append_bool %uint_0 {{%[0-9]+}}

// CHECK:       [[p_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %consume_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[i_0:%[0-9]+]] = OpLoad %uint [[p_1]]
// CHECK-NEXT:                 OpStore [[p_0]] [[i_0]]
  append_bool.Append(consume_bool.Consume());

// CHECK:       [[p_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %consume_int %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[i_2:%[0-9]+]] = OpLoad %int [[p_2]]
// CHECK-NEXT:  [[b_2:%[0-9]+]] = OpINotEqual %bool [[i_2]] %int_0
// CHECK-NEXT: [[bi_2:%[0-9]+]] = OpSelect %uint [[b_2]] %uint_1 %uint_0
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[bi_2]]
  append_bool.Append(consume_int.Consume());

// CHECK:       [[p_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %consume_float %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[f_3:%[0-9]+]] = OpLoad %float [[p_3]]
// CHECK-NEXT:  [[b_3:%[0-9]+]] = OpFOrdNotEqual %bool [[f_3]] %float_0
// CHECK-NEXT: [[bi_3:%[0-9]+]] = OpSelect %uint [[b_3]] %uint_1 %uint_0
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[bi_3]]
  append_bool.Append(consume_float.Consume());

// CHECK:       [[p_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %append_bool %uint_0 {{%[0-9]+}}

// CHECK:       [[p_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[p_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[p_5]] %int_3
// CHECK-NEXT:  [[i_5:%[0-9]+]] = OpLoad %uint [[p_6]]
// CHECK-NEXT: [[bi_5:%[0-9]+]] = OpINotEqual %bool [[i_5]] %uint_0
// CHECK-NEXT:  [[i_5_0:%[0-9]+]] = OpSelect %uint [[bi_5]] %uint_1 %uint_0
// CHECK-NEXT:                 OpStore [[p_4]] [[i_5_0]]
  append_bool.Append(consume_struct_with_bool.Consume().elem_bool);

  //
  // TODO(jaebaek): Uncomment this and all other commented lines after fixing type cast bug
  // https://github.com/Microsoft/DirectXShaderCompiler/issues/2031
  //
  // append_bool.Append(consume_struct_with_bool.Consume().elem_int);
  // append_bool.Append(consume_struct_with_bool.Consume().elem_float);

// CHECK:       [[p_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %consume_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[i_7:%[0-9]+]] = OpLoad %uint [[p_7]]
// CHECK-NEXT:  [[b_7:%[0-9]+]] = OpINotEqual %bool [[i_7]] %uint_0
// CHECK-NEXT: [[bi_7:%[0-9]+]] = OpSelect %int [[b_7]] %int_1 %int_0
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[bi_7]]
  append_int.Append(consume_bool.Consume());

// CHECK:       [[p_8:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %consume_int %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[i_8:%[0-9]+]] = OpLoad %int [[p_8]]
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[i_8]]
  append_int.Append(consume_int.Consume());

// CHECK:       [[p_9:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %consume_float %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[f_9:%[0-9]+]] = OpLoad %float [[p_9]]
// CHECK-NEXT:  [[i_9:%[0-9]+]] = OpConvertFToS %int [[f_9]]
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[i_9]]
  append_int.Append(consume_float.Consume());

  // append_int.Append(consume_struct_with_bool.Consume().elem_bool);

// CHECK:      [[p_10:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int {{%[0-9]+}} %int_4
// CHECK-NEXT: [[i_10:%[0-9]+]] = OpLoad %int [[p_10]]
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[i_10]]
  append_int.Append(consume_struct_with_bool.Consume().elem_int);

  // append_int.Append(consume_struct_with_bool.Consume().elem_float);

// CHECK:      [[p_11:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %consume_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[i_11:%[0-9]+]] = OpLoad %uint [[p_11]]
// CHECK-NEXT: [[b_11:%[0-9]+]] = OpINotEqual %bool [[i_11]] %uint_0
// CHECK-NEXT: [[f_11:%[0-9]+]] = OpSelect %float [[b_11]] %float_1 %float_0
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[f_11]]
  append_float.Append(consume_bool.Consume());

// CHECK:      [[p_12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %consume_int %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[i_12:%[0-9]+]] = OpLoad %int [[p_12]]
// CHECK-NEXT: [[f_12:%[0-9]+]] = OpConvertSToF %float [[i_12]]
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[f_12]]
  append_float.Append(consume_int.Consume());

// CHECK:      [[p_13:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %consume_float %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[f_13:%[0-9]+]] = OpLoad %float [[p_13]]
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[f_13]]
  append_float.Append(consume_float.Consume());

  // append_float.Append(consume_struct_with_bool.Consume().elem_bool);
  // append_float.Append(consume_struct_with_bool.Consume().elem_int);

// CHECK:      [[p_14:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[p_15:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[p_14]] %int_5
// CHECK-NEXT: [[f_15:%[0-9]+]] = OpLoad %float [[p_15]]
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[f_15]]
  append_float.Append(consume_struct_with_bool.Consume().elem_float);

// CHECK:       [[p_16:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vu_16:%[0-9]+]] = OpLoad %v2uint [[p_16]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_16]]
  append_v2bool.Append(consume_v2bool.Consume());

// CHECK:       [[p_17:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int %consume_v2int %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vi_17:%[0-9]+]] = OpLoad %v2int [[p_17]]
// CHECK-NEXT: [[vb_17:%[0-9]+]] = OpINotEqual %v2bool [[vi_17]] {{%[0-9]+}}
// CHECK-NEXT: [[vu_17:%[0-9]+]] = OpSelect %v2uint [[vb_17]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_17]]
  append_v2bool.Append(consume_v2int.Consume());

// CHECK:       [[p_18:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float %consume_v2float %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vf_18:%[0-9]+]] = OpLoad %v2float [[p_18]]
// CHECK-NEXT: [[vb_18:%[0-9]+]] = OpFOrdNotEqual %v2bool [[vf_18]] {{%[0-9]+}}
// CHECK-NEXT: [[vu_18:%[0-9]+]] = OpSelect %v2uint [[vb_18]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_18]]
  append_v2bool.Append(consume_v2float.Consume());

// CHECK:       [[p_19:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[p_20:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint [[p_19]] %int_0
// CHECK-NEXT: [[vu_20:%[0-9]+]] = OpLoad %v2uint [[p_20]]
// CHECK-NEXT: [[vb_20:%[0-9]+]] = OpINotEqual %v2bool [[vu_20]] {{%[0-9]+}}
// CHECK-NEXT: [[vu_20_0:%[0-9]+]] = OpSelect %v2uint [[vb_20]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_20_0]]
  append_v2bool.Append(consume_struct_with_bool.Consume().elem_v2bool);

// CHECK:       [[p_21:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK:       [[p_22:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int [[p_21]] %int_1
// CHECK-NEXT: [[vi_22:%[0-9]+]] = OpLoad %v2int [[p_22]]
// CHECK-NEXT: [[vb_22:%[0-9]+]] = OpINotEqual %v2bool [[vi_22]] {{%[0-9]+}}
// CHECK-NEXT: [[vu_22:%[0-9]+]] = OpSelect %v2uint [[vb_22]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_22]]
  append_v2bool.Append(consume_struct_with_bool.Consume().elem_v2int);

// CHECK:       [[p_23:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK:       [[p_24:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float [[p_23]] %int_2
// CHECK-NEXT: [[vf_24:%[0-9]+]] = OpLoad %v2float [[p_24]]
// CHECK-NEXT: [[vb_24:%[0-9]+]] = OpFOrdNotEqual %v2bool [[vf_24]] {{%[0-9]+}}
// CHECK-NEXT: [[vu_24:%[0-9]+]] = OpSelect %v2uint [[vb_24]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_24]]
  append_v2bool.Append(consume_struct_with_bool.Consume().elem_v2float);

// CHECK:       [[p_25:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vu_25:%[0-9]+]] = OpLoad %v2uint [[p_25]]
// CHECK-NEXT: [[vb_25:%[0-9]+]] = OpINotEqual %v2bool [[vu_25]] {{%[0-9]+}}
// CHECK-NEXT: [[vu_25_0:%[0-9]+]] = OpSelect %v2int [[vb_25]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vu_25_0]]
  append_v2int.Append(consume_v2bool.Consume());

// CHECK:       [[p_26:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int %consume_v2int %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vi_26:%[0-9]+]] = OpLoad %v2int [[p_26]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vi_26]]
  append_v2int.Append(consume_v2int.Consume());

// CHECK:       [[p_27:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float %consume_v2float %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vf_27:%[0-9]+]] = OpLoad %v2float [[p_27]]
// CHECK-NEXT: [[vi_27:%[0-9]+]] = OpConvertFToS %v2int [[vf_27]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vi_27]]
  append_v2int.Append(consume_v2float.Consume());

// CHECK:       [[p_28:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint {{%[0-9]+}} %int_0
// CHECK-NEXT: [[vu_28:%[0-9]+]] = OpLoad %v2uint [[p_28]]
// CHECK-NEXT: [[vb_28:%[0-9]+]] = OpINotEqual %v2bool [[vu_28]] {{%[0-9]+}}
// CHECK-NEXT: [[vi_28:%[0-9]+]] = OpSelect %v2int [[vb_28]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vi_28]]
  append_v2int.Append(consume_struct_with_bool.Consume().elem_v2bool);

// CHECK:       [[p_29:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int {{%[0-9]+}} %int_1
// CHECK-NEXT: [[vi_29:%[0-9]+]] = OpLoad %v2int [[p_29]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vi_29]]
  append_v2int.Append(consume_struct_with_bool.Consume().elem_v2int);

// CHECK:       [[p_30:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float {{%[0-9]+}} %int_2
// CHECK-NEXT: [[vf_30:%[0-9]+]] = OpLoad %v2float [[p_30]]
// CHECK-NEXT: [[vi_30:%[0-9]+]] = OpConvertFToS %v2int [[vf_30]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vi_30]]
  append_v2int.Append(consume_struct_with_bool.Consume().elem_v2float);

// CHECK:       [[p_31:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vu_31:%[0-9]+]] = OpLoad %v2uint [[p_31]]
// CHECK-NEXT: [[vb_31:%[0-9]+]] = OpINotEqual %v2bool [[vu_31]] {{%[0-9]+}}
// CHECK-NEXT: [[vf_31:%[0-9]+]] = OpSelect %v2float [[vb_31]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vf_31]]
  append_v2float.Append(consume_v2bool.Consume());

// CHECK:       [[p_32:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int %consume_v2int %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vi_32:%[0-9]+]] = OpLoad %v2int [[p_32]]
// CHECK-NEXT: [[vf_32:%[0-9]+]] = OpConvertSToF %v2float [[vi_32]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vf_32]]
  append_v2float.Append(consume_v2int.Consume());

// CHECK:       [[p_33:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float %consume_v2float %uint_0 {{%[0-9]+}}
// CHECK-NEXT: [[vf_33:%[0-9]+]] = OpLoad %v2float [[p_33]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vf_33]]
  append_v2float.Append(consume_v2float.Consume());

// CHECK:       [[p_34:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint {{%[0-9]+}} %int_0
// CHECK-NEXT: [[vu_34:%[0-9]+]] = OpLoad %v2uint [[p_34]]
// CHECK-NEXT: [[vb_34:%[0-9]+]] = OpINotEqual %v2bool [[vu_34]] {{%[0-9]+}}
// CHECK-NEXT: [[vf_34:%[0-9]+]] = OpSelect %v2float [[vb_34]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vf_34]]
  append_v2float.Append(consume_struct_with_bool.Consume().elem_v2bool);

// CHECK:       [[p_35:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int {{%[0-9]+}} %int_1
// CHECK-NEXT: [[vi_35:%[0-9]+]] = OpLoad %v2int [[p_35]]
// CHECK-NEXT: [[vf_35:%[0-9]+]] = OpConvertSToF %v2float [[vi_35]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vf_35]]
  append_v2float.Append(consume_struct_with_bool.Consume().elem_v2int);

// CHECK:       [[p_36:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float {{%[a-zA-Z0-9_]+}} %int_2
// CHECK-NEXT: [[vf_36:%[0-9]+]] = OpLoad %v2float [[p_36]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[vf_36]]
  append_v2float.Append(consume_struct_with_bool.Consume().elem_v2float);

// CHECK:       [[p_37:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2uint %consume_v2bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[p_38:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[p_37]] %int_0
// CHECK-NEXT:  [[i_38:%[0-9]+]] = OpLoad %uint [[p_38]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[i_38]]
  append_bool.Append(consume_v2bool.Consume().x);

// CHECK:       [[p_39:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[i_39:%[0-9]+]] = OpLoad %int [[p_39]]
// CHECK-NEXT:  [[b_39:%[0-9]+]] = OpINotEqual %bool [[i_39]] %int_0
// CHECK-NEXT: [[bi_39:%[0-9]+]] = OpSelect %uint [[b_39]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[bi_39]]
  append_bool.Append(consume_v2int.Consume().x);

// CHECK:       [[p_40:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[f_40:%[0-9]+]] = OpLoad %float [[p_40]]
// CHECK-NEXT:  [[b_40:%[0-9]+]] = OpFOrdNotEqual %bool [[f_40]] %float_0
// CHECK-NEXT: [[bi_40:%[0-9]+]] = OpSelect %uint [[b_40]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[bi_40]]
  append_bool.Append(consume_v2float.Consume().x);

// CHECK:       [[p_41:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[i_41:%[0-9]+]] = OpLoad %uint [[p_41]]
// CHECK-NEXT:  [[b_41:%[0-9]+]] = OpINotEqual %bool [[i_41]] %uint_0
// CHECK-NEXT:  [[i_41_0:%[0-9]+]] = OpSelect %uint [[b_41]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[i_41_0]]
  append_bool.Append(consume_struct_with_bool.Consume().elem_v2bool.x);

  // append_bool.Append(consume_struct_with_bool.Consume().elem_v2int.x);
  // append_bool.Append(consume_struct_with_bool.Consume().elem_v2float.x);

// CHECK:       [[p_42:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[i_42:%[0-9]+]] = OpLoad %uint [[p_42]]
// CHECK-NEXT:  [[b_42:%[0-9]+]] = OpINotEqual %bool [[i_42]] %uint_0
// CHECK-NEXT: [[bi_42:%[0-9]+]] = OpSelect %int [[b_42]] %int_1 %int_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[bi_42]]
  append_int.Append(consume_v2bool.Consume().x);

// CHECK:       [[p_43:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[i_43:%[0-9]+]] = OpLoad %int [[p_43]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[i_43]]
  append_int.Append(consume_v2int.Consume().x);

// CHECK:       [[p_44:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[f_44:%[0-9]+]] = OpLoad %float [[p_44]]
// CHECK-NEXT:  [[i_44:%[0-9]+]] = OpConvertFToS %int [[f_44]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[i_44]]
  append_int.Append(consume_v2float.Consume().x);

  // append_int.Append(consume_struct_with_bool.Consume().elem_v2bool.x);

// CHECK:       [[p_45:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[p_46:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2int [[p_45]] %int_1
// CHECK-NEXT:  [[p_47:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[p_46]] %int_0
// CHECK-NEXT:  [[i_47:%[0-9]+]] = OpLoad %int [[p_47]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[i_47]]
  append_int.Append(consume_struct_with_bool.Consume().elem_v2int.x);

  // append_int.Append(consume_struct_with_bool.Consume().elem_v2float.x);

// CHECK:       [[p_48:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[i_48:%[0-9]+]] = OpLoad %uint [[p_48]]
// CHECK-NEXT:  [[b_48:%[0-9]+]] = OpINotEqual %bool [[i_48]] %uint_0
// CHECK-NEXT:  [[f_48:%[0-9]+]] = OpSelect %float [[b_48]] %float_1 %float_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[f_48]]
  append_float.Append(consume_v2bool.Consume().x);

// CHECK:       [[p_49:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[i_49:%[0-9]+]] = OpLoad %int [[p_49]]
// CHECK-NEXT:  [[f_49:%[0-9]+]] = OpConvertSToF %float [[i_49]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[f_49]]
  append_float.Append(consume_v2int.Consume().x);

// CHECK:       [[p_50:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float {{%[0-9]+}} %int_0
// CHECK-NEXT:  [[f_50:%[0-9]+]] = OpLoad %float [[p_50]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[f_50]]
  append_float.Append(consume_v2float.Consume().x);

  // append_float.Append(consume_struct_with_bool.Consume().elem_v2bool.x);
  // append_float.Append(consume_struct_with_bool.Consume().elem_v2int.x);

// CHECK:       [[p_51:%[0-9]+]] = OpAccessChain %_ptr_Uniform_struct_with_bool %consume_struct_with_bool %uint_0 {{%[0-9]+}}
// CHECK-NEXT:  [[p_52:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float [[p_51]] %int_2
// CHECK-NEXT:  [[p_53:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[p_52]] %int_0
// CHECK-NEXT:  [[f_53:%[0-9]+]] = OpLoad %float [[p_53]]
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[f_53]]
  append_float.Append(consume_struct_with_bool.Consume().elem_v2float.x);

// CHECK:       [[p_54:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %append_bool %uint_0 {{%[0-9]+}}
// CHECK:       [[p_55:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %rw_bool %int_0 %uint_0
// CHECK-NEXT:  [[i_55:%[0-9]+]] = OpLoad %uint [[p_55]]
// CHECK-NEXT:  [[b_55:%[0-9]+]] = OpINotEqual %bool [[i_55]] %uint_0
// CHECK-NEXT: [[bi_55:%[0-9]+]] = OpSelect %uint [[b_55]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore [[p_54]] [[bi_55]]
  append_bool.Append(rw_bool[0]);

// CHECK:       [[i_56:%[0-9]+]] = OpLoad %uint {{%[0-9]+}}
// CHECK-NEXT:  [[b_56:%[0-9]+]] = OpINotEqual %bool [[i_56]] %uint_0
// CHECK-NEXT: [[bi_56:%[0-9]+]] = OpSelect %uint [[b_56]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[bi_56]]
  append_bool.Append(rw_v2bool[0].x);

// CHECK:       [[p_57:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %append_int %uint_0 {{%[0-9]+}}
// CHECK:       [[p_58:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %rw_bool %int_0 %uint_0
// CHECK-NEXT:  [[i_58:%[0-9]+]] = OpLoad %uint [[p_58]]
// CHECK-NEXT:  [[b_58:%[0-9]+]] = OpINotEqual %bool [[i_58]] %uint_0
// CHECK-NEXT: [[bi_58:%[0-9]+]] = OpSelect %int [[b_58]] %int_1 %int_0
// CHECK-NEXT:                  OpStore [[p_57]] [[bi_58]]
  append_int.Append(rw_bool[0]);

// CHECK:       [[i_59:%[0-9]+]] = OpLoad %uint {{%[0-9]+}}
// CHECK-NEXT:  [[b_59:%[0-9]+]] = OpINotEqual %bool [[i_59]] %uint_0
// CHECK-NEXT: [[bi_59:%[0-9]+]] = OpSelect %int [[b_59]] %int_1 %int_0
// CHECK-NEXT:                 OpStore {{%[0-9]+}} [[bi_59]]
  append_int.Append(rw_v2bool[0].x);

// CHECK:      [[p_60:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %append_float %uint_0 {{%[0-9]+}}
// CHECK:      [[p_61:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %rw_bool %int_0 %uint_0
// CHECK-NEXT: [[i_61:%[0-9]+]] = OpLoad %uint [[p_61]]
// CHECK-NEXT: [[b_61:%[0-9]+]] = OpINotEqual %bool [[i_61]] %uint_0
// CHECK-NEXT: [[f_61:%[0-9]+]] = OpSelect %float [[b_61]] %float_1 %float_0
// CHECK-NEXT:                 OpStore [[p_60]] [[f_61]]
  append_float.Append(rw_bool[0]);

// CHECK:       [[i_61_0:%[0-9]+]] = OpLoad %uint {{%[0-9]+}}
// CHECK-NEXT:  [[b_61_0:%[0-9]+]] = OpINotEqual %bool [[i_61_0]] %uint_0
// CHECK-NEXT: [[bi_61:%[0-9]+]] = OpSelect %float [[b_61_0]] %float_1 %float_0
// CHECK-NEXT:                  OpStore {{%[0-9]+}} [[bi_61]]
  append_float.Append(rw_v2bool[0].x);

// CHECK:      [[matPtr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v2float %consume_float2x2 %uint_0 {{%[0-9]+}}
// CHECK-NEXT:    [[mat:%[0-9]+]] = OpLoad %mat2v2float [[matPtr]]
// CHECK-NEXT:   [[row0:%[0-9]+]] = OpCompositeExtract %v2float [[mat]] 0
// CHECK-NEXT:   [[row1:%[0-9]+]] = OpCompositeExtract %v2float [[mat]] 1
// CHECK-NEXT:   [[vec4:%[0-9]+]] = OpVectorShuffle %v4float [[row0]] [[row1]] 0 1 2 3
// CHECK-NEXT:                   OpStore {{%[0-9]+}} [[vec4]]
  append_v4float.Append(consume_float2x2.Consume());
}
