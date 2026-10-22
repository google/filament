// RUN: %dxc -T ps_6_2 -enable-16bit-types -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<half> buffer;
RWStructuredBuffer<half2x3> buffer_mat;
RWByteAddressBuffer byte_buffer;

void main() {
  half    a;
  half4   b;
  half2x3 c;

// CHECK:      [[a:%[0-9]+]] = OpLoad %half %a
// CHECK-NEXT:   {{%[0-9]+}} = OpIsNan %bool [[a]]
  bool  isnan_a = isnan(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4half %b
// CHECK-NEXT:   {{%[0-9]+}} = OpIsNan %v4bool [[b]]
  bool4 isnan_b = isnan(b);

  // TODO: We can not translate the following since boolean matrices are currently not supported.
  // bool2x3 isnan_c = isnan(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %half [[ptr]]
// CHECK:      [[res:%[0-9]+]] = OpIsNan %bool [[tmp]]
// CHECK:                        OpStore %res [[res]]
// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %res
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %half [[res]] %half_0x1p_0 %half_0x0p_0
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_half %buffer %int_0 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  bool res = isnan(buffer[0]);
  buffer[0] = (half)res;

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3half %c
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3half [[c]] 0
// CHECK: [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3half [[c]] 1
// CHECK: [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isnan_r0]] [[isnan_r1]]
// CHECK:                        OpStore %isnan_c [[tmp]]
  bool2x3 isnan_c = isnan(c);

// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat2v3half %buffer_mat %int_0 %uint_0
// CHECK:      [[tmp:%[0-9]+]] = OpLoad %mat2v3half [[ptr]]
// CHECK:       [[r0:%[0-9]+]] = OpCompositeExtract %v3half [[tmp]] 0
// CHECK: [[isnan_r0:%[0-9]+]] = OpIsNan %v3bool [[r0]]
// CHECK:       [[r1:%[0-9]+]] = OpCompositeExtract %v3half [[tmp]] 1
// CHECK: [[isnan_r1:%[0-9]+]] = OpIsNan %v3bool [[r1]]
// CHECK:      [[tmp:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[isnan_r0]] [[isnan_r1]]
// CHECK:                        OpStore %isnan_d [[tmp]]
  bool2x3 isnan_d = isnan(buffer_mat[0]);

// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmpL:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:      [[tmpS:%[0-9]+]] = OpShiftRightLogical %uint [[tmpL]] %80
// CHECK:      [[tmp:%[0-9]+]] = OpUConvert %ushort [[tmpS]]
// CHECK:      [[val:%[0-9]+]] = OpBitcast %half [[tmp]]
// CHECK:      [[res:%[0-9]+]] = OpIsNan %bool [[val]]
// CHECK:                        OpStore %isnan_e [[res]]
  bool isnan_e = isnan(byte_buffer.Load<half>(0));

// CHECK:      [[res:%[0-9]+]] = OpLoad %bool %isnan_e
// CHECK:     [[addr:%[0-9]+]] = OpShiftRightLogical %uint %uint_0 %uint_2
// CHECK:      [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %byte_buffer %uint_0 [[addr]]
// CHECK:      [[tmp:%[0-9]+]] = OpSelect %uint [[res]] %uint_1 %uint_0
// CHECK:                        OpStore [[ptr]] [[tmp]]
  byte_buffer.Store(0, isnan_e);
}
