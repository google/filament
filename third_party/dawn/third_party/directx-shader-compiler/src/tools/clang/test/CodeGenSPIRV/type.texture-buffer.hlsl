// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpName %type_TextureBuffer_T "type.TextureBuffer.T"
// CHECK-NEXT: OpMemberName %type_TextureBuffer_T 0 "a"
// CHECK-NEXT: OpMemberName %type_TextureBuffer_T 1 "b"
// CHECK-NEXT: OpMemberName %type_TextureBuffer_T 2 "c"
// CHECK-NEXT: OpMemberName %type_TextureBuffer_T 3 "d"
// CHECK-NEXT: OpMemberName %type_TextureBuffer_T 4 "s"
// CHECK-NEXT: OpMemberName %type_TextureBuffer_T 5 "t"

// CHECK:      OpName %MyTbuffer "MyTbuffer"
// CHECK:      OpName %AnotherTBuffer "AnotherTBuffer"

// CHECK:      OpDecorate %type_TextureBuffer_T BufferBlock

struct S {
  float  f1;
  float3 f2;
};

// CHECK: %type_TextureBuffer_T = OpTypeStruct %uint %int %v2uint %mat3v4float %S %_arr_float_uint_4
struct T {
  bool     a;
  int      b;
  uint2    c;
  float3x4 d;
  S        s;
  float    t[4];
};

// CHECK: %_ptr_Uniform_type_TextureBuffer_T = OpTypePointer Uniform %type_TextureBuffer_T
// CEHCK: %_arr_type_TextureBuffer_T_uint_3 = OpTypeArray %type_TextureBuffer_T %uint_3
// CEHCK: %_ptr_Uniform__arr_type_TextureBuffer_T_uint_3 = OpTypePointer Uniform %_arr_type_TextureBuffer_T_uint_3

// CHECK: %MyTbuffer = OpVariable %_ptr_Uniform_type_TextureBuffer_T Uniform
TextureBuffer<T> MyTbuffer : register(t1);

// CHECK: %AnotherTBuffer = OpVariable %_ptr_Uniform_type_TextureBuffer_T Uniform
TextureBuffer<T> AnotherTBuffer : register(t2);

// CHECK: %myTextureBufferArray = OpVariable %_ptr_Uniform__arr_type_TextureBuffer_T_uint_3 Uniform
TextureBuffer<T> myTextureBufferArray[3] : register(t3);

void main() {
}
