// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'modf' function can only operate on scalar/vector/matrix of float/int.

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

// CHECK: OpName %ModfStructType "ModfStructType"
// CHECK: OpMemberName %ModfStructType 0 "frac"
// CHECK: OpMemberName %ModfStructType 1 "ip"
// CHECK: OpName %ModfStructType_0 "ModfStructType"
// CHECK: OpMemberName %ModfStructType_0 0 "frac"
// CHECK: OpMemberName %ModfStructType_0 1 "ip"
// CHECK: OpName %ModfStructType_1 "ModfStructType"
// CHECK: OpMemberName %ModfStructType_1 0 "frac"
// CHECK: OpMemberName %ModfStructType_1 1 "ip"

// Note: Even though we have used non-float types below,
// DXC has casted the input of modf() into float. Therefore
// all struct types have float members.
// CHECK:   %ModfStructType = OpTypeStruct %float %float
// CHECK: %ModfStructType_0 = OpTypeStruct %v4float %v4float
// CHECK: %ModfStructType_1 = OpTypeStruct %v3float %v3float

void main() {
  uint     a, ip_a, frac_a;
  int4     b, ip_b, frac_b;
  float2x3 c, ip_c, frac_c;
  float2x3 d;
  int2x3   frac_d, ip_d;

// CHECK:                 [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT:           [[af:%[0-9]+]] = OpConvertUToF %float [[a]]
// CHECK-NEXT:[[modf_struct_a:%[0-9]+]] = OpExtInst %ModfStructType [[glsl]] ModfStruct [[af]]
// CHECK-NEXT:         [[ip_a:%[0-9]+]] = OpCompositeExtract %float [[modf_struct_a]] 1
// CHECK-NEXT:    [[uint_ip_a:%[0-9]+]] = OpConvertFToU %uint [[ip_a]]
// CHECK-NEXT:                         OpStore %ip_a [[uint_ip_a]]
// CHECK-NEXT:       [[frac_a:%[0-9]+]] = OpCompositeExtract %float [[modf_struct_a]] 0
// CHECK-NEXT:  [[uint_frac_a:%[0-9]+]] = OpConvertFToU %uint [[frac_a]]
// CHECK-NEXT:                         OpStore %frac_a [[uint_frac_a]]
  frac_a = modf(a, ip_a);

// CHECK:                 [[b:%[0-9]+]] = OpLoad %v4int %b
// CHECK-NEXT:           [[bf:%[0-9]+]] = OpConvertSToF %v4float [[b]]
// CHECK-NEXT:[[modf_struct_b:%[0-9]+]] = OpExtInst %ModfStructType_0 [[glsl]] ModfStruct [[bf]]
// CHECK-NEXT:         [[ip_b:%[0-9]+]] = OpCompositeExtract %v4float [[modf_struct_b]] 1
// CHECK-NEXT:    [[int4_ip_b:%[0-9]+]] = OpConvertFToS %v4int [[ip_b]]
// CHECK-NEXT:                         OpStore %ip_b [[int4_ip_b]]
// CHECK-NEXT:       [[frac_b:%[0-9]+]] = OpCompositeExtract %v4float [[modf_struct_b]] 0
// CHECK-NEXT:  [[int4_frac_b:%[0-9]+]] = OpConvertFToS %v4int [[frac_b]]
// CHECK-NEXT:                         OpStore %frac_b [[int4_frac_b]]
  frac_b = modf(b, ip_b);

// CHECK:                      [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:            [[c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT:[[modf_struct_c_row0:%[0-9]+]] = OpExtInst %ModfStructType_1 [[glsl]] ModfStruct [[c_row0]]
// CHECK-NEXT:         [[ip_c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_c_row0]] 1
// CHECK-NEXT:       [[frac_c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_c_row0]] 0
// CHECK-NEXT:            [[c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT:[[modf_struct_c_row1:%[0-9]+]] = OpExtInst %ModfStructType_1 [[glsl]] ModfStruct [[c_row1]]
// CHECK-NEXT:         [[ip_c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_c_row1]] 1
// CHECK-NEXT:       [[frac_c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_c_row1]] 0
// CHECK-NEXT:              [[ip_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[ip_c_row0]] [[ip_c_row1]]
// CHECK-NEXT:                              OpStore %ip_c [[ip_c]]
// CHECK-NEXT:            [[frac_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[frac_c_row0]] [[frac_c_row1]]
// CHECK-NEXT:                              OpStore %frac_c [[frac_c]]
  frac_c = modf(c, ip_c);

// CHECK:                       [[d:%[0-9]+]] = OpLoad %mat2v3float %d
// CHECK-NEXT:             [[d_row0:%[0-9]+]] = OpCompositeExtract %v3float [[d]] 0
// CHECK-NEXT: [[modf_struct_d_row0:%[0-9]+]] = OpExtInst %ModfStructType_1 [[glsl]] ModfStruct [[d_row0]]
// CHECK-NEXT:          [[ip_d_row0:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_d_row0]] 1
// CHECK-NEXT:        [[frac_d_row0:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_d_row0]] 0
// CHECK-NEXT:             [[d_row1:%[0-9]+]] = OpCompositeExtract %v3float [[d]] 1
// CHECK-NEXT: [[modf_struct_d_row1:%[0-9]+]] = OpExtInst %ModfStructType_1 [[glsl]] ModfStruct [[d_row1]]
// CHECK-NEXT:          [[ip_d_row1:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_d_row1]] 1
// CHECK-NEXT:        [[frac_d_row1:%[0-9]+]] = OpCompositeExtract %v3float [[modf_struct_d_row1]] 0
// CHECK-NEXT:       [[ip_float_mat:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[ip_d_row0]] [[ip_d_row1]]
// CHECK-NEXT:  [[ip_float_mat_row0:%[0-9]+]] = OpCompositeExtract %v3float [[ip_float_mat]] 0
// CHECK-NEXT:    [[ip_int_mat_row0:%[0-9]+]] = OpConvertFToS %v3int [[ip_float_mat_row0]]
// CHECK-NEXT:  [[ip_float_mat_row1:%[0-9]+]] = OpCompositeExtract %v3float [[ip_float_mat]] 1
// CHECK-NEXT:    [[ip_int_mat_row1:%[0-9]+]] = OpConvertFToS %v3int [[ip_float_mat_row1]]
// CHECK-NEXT:         [[ip_int_mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[ip_int_mat_row0]] [[ip_int_mat_row1]]
// CHECK-NEXT:                               OpStore %ip_d [[ip_int_mat]]
// CHECK-NEXT:     [[frac_float_mat:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[frac_d_row0]] [[frac_d_row1]]
// CHECK-NEXT:[[frac_float_mat_row0:%[0-9]+]] = OpCompositeExtract %v3float [[frac_float_mat]] 0
// CHECK-NEXT:  [[frac_int_mat_row0:%[0-9]+]] = OpConvertFToS %v3int [[frac_float_mat_row0]]
// CHECK-NEXT:[[frac_float_mat_row1:%[0-9]+]] = OpCompositeExtract %v3float [[frac_float_mat]] 1
// CHECK-NEXT:  [[frac_int_mat_row1:%[0-9]+]] = OpConvertFToS %v3int [[frac_float_mat_row1]]
// CHECK-NEXT:       [[frac_int_mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[frac_int_mat_row0]] [[frac_int_mat_row1]]
// CHECK-NEXT:                               OpStore %frac_d [[frac_int_mat]]
  frac_d = modf(d, ip_d);
}
