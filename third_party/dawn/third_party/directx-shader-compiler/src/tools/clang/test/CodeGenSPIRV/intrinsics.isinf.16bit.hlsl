// RUN: %dxc -T ps_6_2 -enable-16bit-types -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<half> buffer;
RWStructuredBuffer<half2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

void main() {
  half    a;
  half4   b;
  half2x3 c;

// CHECK:      [[a:%[0-9]+]] = OpLoad %half %a
// CHECK-NEXT:   {{%[0-9]+}} = OpIsInf %bool [[a]]
  bool  isinf_a = isinf(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4half %b
// CHECK-NEXT:   {{%[0-9]+}} = OpIsInf %v4bool [[b]]
  bool4 isinf_b = isinf(b);

  // TODO: We can not translate the following since boolean matrices are currently not supported.
  // bool2x3 isinf_c = isinf(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %half [[ptr]]
// CHECK:      [[res:%[0-9]+]] = OpIsInf %bool [[tmp]]
// CHECK:                        OpStore %res [[res]]
// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %res
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %half [[res]] %half_0x1p_0 %half_0x0p_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  bool res = isinf(buffer[0]);
  buffer[0] = (half)res;

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3half %c
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3half [[c]] 0
// CHECK: [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3half [[c]] 1
// CHECK: [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isinf_r0]] [[isinf_r1]]
// CHECK:                        OpStore %isinf_c [[tmp]]
  bool2x3 isinf_c = isinf(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3half %buffer_mat %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %mat2v3half [[ptr]]
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3half [[tmp]] 0
// CHECK: [[isinf_r0:%[0-9]+]] = OpIsInf %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3half [[tmp]] 1
// CHECK: [[isinf_r1:%[0-9]+]] = OpIsInf %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isinf_r0]] [[isinf_r1]]
// CHECK:                        OpStore %isinf_d [[tmp]]
  bool2x3 isinf_d = isinf(buffer_mat[0]);

// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmpL:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:      [[tmpS:%[0-9]+]] = OpShiftRightLogical %uint [[tmpL]] %80
// CHECK:      [[tmp:%[0-9]+]] = OpUConvert %ushort [[tmpS]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %half [[tmp]]
// CHECK:      [[res:%[0-9]+]] = OpIsInf %bool [[val]]
// CHECK:                        OpStore %isinf_e [[res]]
  bool isinf_e = isinf(byte_buffer.Load<half>(0));

// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %isinf_e
// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %uint [[res]] %uint_1 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  byte_buffer.Store(0, isinf_e);
}
