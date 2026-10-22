// RUN: %dxc -T cs_6_2 -E main %s -fvk-use-scalar-layout -spirv | FileCheck %s

// Check that arrays of structs respect alignment requirements. Note that this
// test does not distinguish between C-style and "strict" scalar layout.

// CHECK-DAG: OpMemberDecorate %Foo 0 Offset 0
// CHECK-DAG: OpMemberDecorate %Foo 1 Offset 8
// CHECK-DAG: OpDecorate %_arr_Foo_uint_3 ArrayStride 16
// CHECK-DAG: OpMemberDecorate %Bar 0 Offset 0
// CHECK-DAG: OpDecorate %_runtimearr_Bar ArrayStride 56
// CHECK-DAG: OpMemberDecorate %type_RWStructuredBuffer_Bar 0 Offset 0

struct Foo {
  uint64_t a;
  int b;
};

struct Bar {
  Foo foo[3];
  uint64_t c;
};

RWStructuredBuffer<Bar> buffer;

[numthreads(1, 1, 1)] void main() {
  // CHECK:      [[buf_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_ulong %buffer %int_0 %uint_0 %int_1
  // CHECK-NEXT:                     OpStore [[buf_0]] %ulong_56
  buffer[0].c = sizeof(Bar);
}
