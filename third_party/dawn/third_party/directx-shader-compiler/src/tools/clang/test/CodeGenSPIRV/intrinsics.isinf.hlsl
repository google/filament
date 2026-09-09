// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<float> buffer;
RWStructuredBuffer<float2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

void main() {
  float    a;
  float4   b;
  float2x3 c;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:   {{%[0-9]+}} = OpIsInf %bool [[a]]
  bool  isinf_a = isinf(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:   {{%[0-9]+}} = OpIsInf %v4bool [[b]]
  bool4 isinf_b = isinf(b);

  // TODO: We can not translate the following since boolean matrices are currently not supported.
  // bool2x3 isinf_c = isinf(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %buffer %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %float [[ptr]]
// CHECK:      [[res:%[0-9]+]] = OpIsInf %bool [[tmp]]
// CHECK:                        OpStore %res [[res]]
// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %res
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %float [[res]] %float_1 %float_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %buffer %int_0 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  bool res = isinf(buffer[0]);
  buffer[0] = (float)res;

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK: [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK: [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isinf_r0]] [[isinf_r1]]
// CHECK:                        OpStore %isinf_c [[tmp]]
  bool2x3 isinf_c = isinf(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3float %buffer_mat %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %mat2v3float [[ptr]]
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3float [[tmp]] 0
// CHECK: [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3float [[tmp]] 1
// CHECK: [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isinf_r0]] [[isinf_r1]]
// CHECK:                        OpStore %isinf_d [[tmp]]
  bool2x3 isinf_d = isinf(buffer_mat[0]);

// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %float [[tmp]]
// CHECK:      [[res:%[0-9]+]] = OpIsInf %bool [[val]]
// CHECK:                        OpStore %isinf_e [[res]]
  bool isinf_e = isinf(byte_buffer.Load<float>(0));

// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %isinf_e
// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %uint [[res]] %uint_1 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  byte_buffer.Store(0, isinf_e);
}
