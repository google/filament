// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<float> buffer;
RWStructuredBuffer<float2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

// Since OpIsFinite needs the Kernel capability, translation is done using OpIsNan and OpIsInf.
// isFinite = !(isNan || isInf)

void main() {
  float    a;
  float4   b;
  float2x3 c;

// CHECK:               [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:    [[a_isNan:%[0-9]+]] = OpIsNan %bool [[a]]
// CHECK-NEXT:    [[a_isInf:%[0-9]+]] = OpIsInf %bool [[a]]
// CHECK-NEXT: [[a_NanOrInf:%[0-9]+]] = OpLogicalOr %bool [[a_isNan]] [[a_isInf]]
// CHECK-NEXT:            {{%[0-9]+}} = OpLogicalNot %bool [[a_NanOrInf]]
  bool    isf_a = isfinite(a);

// CHECK:                [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:     [[b_isNan:%[0-9]+]] = OpIsNan %v4bool [[b]]
// CHECK-NEXT:     [[b_isInf:%[0-9]+]] = OpIsInf %v4bool [[b]]
// CHECK-NEXT:  [[b_NanOrInf:%[0-9]+]] = OpLogicalOr %v4bool [[b_isNan]] [[b_isInf]]
// CHECK-NEXT:             {{%[0-9]+}} = OpLogicalNot %v4bool [[b_NanOrInf]]
  bool4   isf_b = isfinite(b);

  // TODO: We can not translate the following since boolean matrices are currently not supported.
  // bool2x3 isf_c = isfinite(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %buffer %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %float [[ptr]]
// CHECK:      [[nan:%[0-9]+]] = OpIsNan %bool [[tmp]]
// CHECK:      [[inf:%[0-9]+]] = OpIsInf %bool [[tmp]]
// CHECK:       [[or:%[0-9]+]] = OpLogicalOr %bool [[nan]] [[inf]]
// CHECK:      [[res:%[0-9]+]] = OpLogicalNot %bool [[or]]
// CHECK:                        OpStore %res [[res]]
// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %res
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %float [[res]] %float_1 %float_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %buffer %int_0 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  bool res = isfinite(buffer[0]);
  buffer[0] = (float)res;

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK: [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK: [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:    [[or_r0:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r0]] [[isinf_r0]]
// CHECK:   [[not_r0:%[0-9]+]] = OpLogicalNot %v3bool [[or_r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK: [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK: [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:    [[or_r1:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r1]] [[isinf_r1]]
// CHECK:   [[not_r1:%[0-9]+]] = OpLogicalNot %v3bool [[or_r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[not_r0]] [[not_r1]]
// CHECK:                        OpStore %isfinite_c [[tmp]]
  bool2x3 isfinite_c = isfinite(c);

// CHECK:         [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3float %buffer_mat %int_0 %uint_0
// CHECK:         [[tmp:%[0-9]+]] = OpLoad %mat2v3float [[ptr]]
// CHECK:          [[r0:%[0-9]+]] = OpCompositeExtract %v3float [[tmp]] 0
// CHECK:    [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK:    [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:       [[or_r0:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r0]] [[isinf_r0]]
// CHECK: [[isfinite_r0:%[0-9]+]] = OpLogicalNot %v3bool [[or_r0]]
// CHECK:          [[r1:%[0-9]+]] = OpCompositeExtract %v3float [[tmp]] 1
// CHECK:    [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK:    [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:       [[or_r1:%[0-9]+]] = OpLogicalOr %v3bool [[isnan_r1]] [[isinf_r1]]
// CHECK: [[isfinite_r1:%[0-9]+]] = OpLogicalNot %v3bool [[or_r1]]
// CHECK:         [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isfinite_r0]] [[isfinite_r1]]
// CHECK:                           OpStore %isfinite_d [[tmp]]
  bool2x3 isfinite_d = isfinite(buffer_mat[0]);

// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %float [[tmp]]
// CHECK:      [[nan:%[0-9]+]] = OpIsNan %bool [[val]]
// CHECK:      [[inf:%[0-9]+]] = OpIsInf %bool [[val]]
// CHECK:       [[or:%[0-9]+]] = OpLogicalOr %bool [[nan]] [[inf]]
// CHECK:      [[res:%[0-9]+]] = OpLogicalNot %bool [[or]]
// CHECK:                        OpStore %isfinite_e [[res]]
  bool isfinite_e = isfinite(byte_buffer.Load<float>(0));

// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %isfinite_e
// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %uint [[res]] %uint_1 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  byte_buffer.Store(0, isfinite_e);
}
