// RUN: %dxc -T cs_6_2 -E main %s -fvk-use-scalar-layout -spirv | FileCheck %s

// Check that the size of Foo and Bar gets rounded up to its alignment to follow
// C-style layout rules. See #7894

// CHECK-DAG: OpMemberDecorate %Foo 0 Offset 0
// CHECK-DAG: OpMemberDecorate %Foo 1 Offset 8
// CHECK-DAG: OpMemberDecorate %Bar 0 Offset 0
// CHECK-DAG: OpMemberDecorate %Bar 1 Offset 16
// CHECK-DAG: OpDecorate %_runtimearr_Bar ArrayStride 24
// CHECK-DAG: OpMemberDecorate %type_RWStructuredBuffer_Bar 0 Offset 0

struct Foo {
  uint64_t a;
  int b;
};

struct Bar {
  Foo foo;
  int c;
};

RWStructuredBuffer<Bar> buffer;

[numthreads(1, 1, 1)]
void main(in uint3 threadId : SV_DispatchThreadID) {
  // CHECK:      [[buf_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buffer %int_0 %uint_0 %int_1
  // CHECK-NEXT:                     OpStore [[buf_0]] %int_16
  buffer[0].c = sizeof(Foo);
  // CHECK:      [[buf_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %buffer %int_0 %uint_1 %int_1
  // CHECK-NEXT:                     OpStore [[buf_1]] %int_24
  buffer[1].c = sizeof(Bar);
}
