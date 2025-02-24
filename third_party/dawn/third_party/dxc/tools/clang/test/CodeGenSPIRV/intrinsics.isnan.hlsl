// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<float> buffer;
RWStructuredBuffer<float2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

void main() {
  float    a;
  float4   b;
  float2x3 c;

// CHECK: %isnan_c = OpVariable %_ptr_Function__arr_v3bool_uint_2 Function

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:   {{%[0-9]+}} = OpIsNan %bool [[a]]
  bool  isnan_a = isnan(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:   {{%[0-9]+}} = OpIsNan %v4bool [[b]]
  bool4 isnan_b = isnan(b);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %buffer %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %float [[ptr]]
// CHECK:      [[res:%[0-9]+]] = OpIsNan %bool [[tmp]]
// CHECK:                        OpStore %res [[res]]
// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %res
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %float [[res]] %float_1 %float_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %buffer %int_0 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  bool res = isnan(buffer[0]);
  buffer[0] = (float)res;

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK: [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK: [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isnan_r0]] [[isnan_r1]]
// CHECK:                        OpStore %isnan_c [[tmp]]
  bool2x3 isnan_c = isnan(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3float %buffer_mat %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %mat2v3float [[ptr]]
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3float [[tmp]] 0
// CHECK: [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3float [[tmp]] 1
// CHECK: [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isnan_r0]] [[isnan_r1]]
// CHECK:                        OpStore %isnan_d [[tmp]]
  bool2x3 isnan_d = isnan(buffer_mat[0]);

// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %float [[tmp]]
// CHECK:      [[res:%[0-9]+]] = OpIsNan %bool [[val]]
// CHECK:                        OpStore %isnan_e [[res]]
  bool isnan_e = isnan(byte_buffer.Load<float>(0));

// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %isnan_e
// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %uint [[res]] %uint_1 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  byte_buffer.Store(0, isnan_e);
}
