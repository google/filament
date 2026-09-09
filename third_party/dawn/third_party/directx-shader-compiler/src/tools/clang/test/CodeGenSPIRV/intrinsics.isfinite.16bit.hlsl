// RUN: %dxc -T ps_6_2 -enable-16bit-types -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<half> buffer;
RWStructuredBuffer<half2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

// Since OpIsFinite needs the Kernel capability, translation is done using OpIsNan and OpIsInf.
// isFinite = !(isNan || isInf)

void main() {
  half    a;
  half4   b;
  half2x3 c;

// CHECK:               [[a:%[0-9]+]] = OpLoad %half %a
// CHECK-NEXT:    [[a_isNan:%[0-9]+]] = OpIsNan %bool [[a]]
// CHECK-NEXT:    [[a_isInf:%[0-9]+]] = OpIsInf %bool [[a]]
// CHECK-NEXT: [[a_NanOrInf:%[0-9]+]] = OpLogicalOr %bool [[a_isNan]] [[a_isInf]]
// CHECK-NEXT:            {{%[0-9]+}} = OpLogicalNot %bool [[a_NanOrInf]]
  bool    isf_a = isfinite(a);

// CHECK:                [[b:%[0-9]+]] = OpLoad %v4half %b
// CHECK-NEXT:     [[b_isNan:%[0-9]+]] = OpIsNan %v4bool [[b]]
// CHECK-NEXT:     [[b_isInf:%[0-9]+]] = OpIsInf %v4bool [[b]]
// CHECK-NEXT:  [[b_NanOrInf:%[0-9]+]] = OpLogicalOr %v4bool [[b_isNan]] [[b_isInf]]
// CHECK-NEXT:             {{%[0-9]+}} = OpLogicalNot %v4bool [[b_NanOrInf]]
  bool4   isf_b = isfinite(b);

  // TODO: We can not translate the following since boolean matrices are currently not supported.
  // bool2x3 isf_c = isfinite(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %half [[ptr]]
// CHECK:      [[nan:%[0-9]+]] = OpIsNan %bool [[tmp]]
// CHECK:      [[inf:%[0-9]+]] = OpIsInf %bool [[tmp]]
// CHECK:       [[or:%[0-9]+]] = OpLogicalOr %bool [[nan]] [[inf]]
// CHECK:      [[res:%[0-9]+]] = OpLogicalNot %bool [[or]]
// CHECK:                        OpStore %res [[res]]
// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %res
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %half [[res]] %half_0x1p_0 %half_0x0p_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  bool res = isfinite(buffer[0]);
  buffer[0] = (half)res;

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3half %c
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3half [[c]] 0
// CHECK: [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK: [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:    [[or_r0:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r0]] [[isinf_r0]]
// CHECK:   [[not_r0:%[0-9]+]] = OpLogicalNot %v3bool [[or_r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3half [[c]] 1
// CHECK: [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK: [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:    [[or_r1:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r1]] [[isinf_r1]]
// CHECK:   [[not_r1:%[0-9]+]] = OpLogicalNot %v3bool [[or_r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[not_r0]] [[not_r1]]
// CHECK:                        OpStore %isfinite_c [[tmp]]
  bool2x3 isfinite_c = isfinite(c);

// CHECK:         [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3half %buffer_mat %int_0 %uint_0
// CHECK:         [[tmp:%[0-9]+]] = OpLoad %mat2v3half [[ptr]]
// CHECK:          [[r0:%[0-9]+]] = OpCompositeExtract %v3half [[tmp]] 0
// CHECK:    [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK:    [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:       [[or_r0:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r0]] [[isinf_r0]]
// CHECK: [[isfinite_r0:%[0-9]+]] = OpLogicalNot %v3bool [[or_r0]]
// CHECK:          [[r1:%[0-9]+]] = OpCompositeExtract %v3half [[tmp]] 1
// CHECK:    [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK:    [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:       [[or_r1:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r1]] [[isinf_r1]]
// CHECK: [[isfinite_r1:%[0-9]+]] = OpLogicalNot %v3bool [[or_r1]]
// CHECK:         [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isfinite_r0]] [[isfinite_r1]]
// CHECK:                           OpStore %isfinite_d [[tmp]]
  bool2x3 isfinite_d = isfinite(buffer_mat[0]);

// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmpL:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:      [[tmpS:%[0-9]+]] = OpShiftRightLogical %uint [[tmpL]] %101
// CHECK:      [[tmp:%[0-9]+]] = OpUConvert %ushort [[tmpS]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %half [[tmp]]
// CHECK:      [[nan:%[0-9]+]] = OpIsNan %bool [[val]]
// CHECK:      [[inf:%[0-9]+]] = OpIsInf %bool [[val]]
// CHECK:       [[or:%[0-9]+]] = OpLogicalOr %bool [[nan]] [[inf]]
// CHECK:      [[res:%[0-9]+]] = OpLogicalNot %bool [[or]]
// CHECK:                        OpStore %isfinite_e [[res]]
  bool isfinite_e = isfinite(byte_buffer.Load<half>(0));

// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %isfinite_e
// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %uint [[res]] %uint_1 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  byte_buffer.Store(0, isfinite_e);
}
