// RUN: %dxc -E main -T vs_6_2 -HV 2018 -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct Empty {};

AppendStructuredBuffer<int> buf;

void main() {
  // Test size and packing of scalar types, vectors and arrays all at once.

  // CHECK:      [[buf:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf]] %int_12
  buf.Append(sizeof(int16_t3[2]));
  // CHECK:      [[buf_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_0]] %int_12
  buf.Append(sizeof(half3[2]));

  // CHECK:      [[buf_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_1]] %int_24
  buf.Append(sizeof(int3[2]));
  // CHECK:      [[buf_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_2]] %int_24
  buf.Append(sizeof(float3[2]));
  // CHECK:      [[buf_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_3]] %int_24
  buf.Append(sizeof(bool3[2]));

  // CHECK:      [[buf_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_4]] %int_48
  buf.Append(sizeof(int64_t3[2]));
  // CHECK:      [[buf_5:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_5]] %int_48
  buf.Append(sizeof(double3[2]));

  // CHECK:      [[buf_6:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_6]] %int_0
  buf.Append(sizeof(Empty[2]));

  // CHECK:      [[buf_7:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buf %uint_0
  // CHECK-NEXT:                OpStore [[buf_7]] %int_8
  struct
  {
    int16_t i16;
    // 2-byte padding
    struct { float f32; } s; // Nested type
    struct {} _; // Zero-sized field.
  } complexStruct;
  buf.Append(sizeof(complexStruct));

// CHECK:         [[foo:%[0-9]+]] = OpLoad %int %foo
// CHECK-NEXT: [[ui_foo:%[0-9]+]] = OpBitcast %uint [[foo]]
// CHECK-NEXT:                   OpIMul %uint %uint_4 [[ui_foo]]
  int foo;
  buf.Append(sizeof(float) * foo);
}
