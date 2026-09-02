// RUN: %dxc -T cs_6_8 -E main %s -spirv -fspv-target-env=vulkan1.3 | FileCheck %s

class A {
  RWStructuredBuffer<uint> a;
};

class B : A {
  RWStructuredBuffer<uint> b;
};

RWStructuredBuffer<uint> input;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
  B x;

  // CHECK:      [[val1_ptr:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_uint %input %int_0 %uint_1
  // CHECK-NEXT: [[val1:%[a-zA-Z0-9_]+]] = OpLoad %uint [[val1_ptr]]
  // CHECK-NEXT: [[val2_ptr:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_uint %output %int_0 %uint_2
  // CHECK-NEXT: [[val2:%[a-zA-Z0-9_]+]] = OpLoad %uint [[val2_ptr]]
  // CHECK-NEXT: [[add_res:%[a-zA-Z0-9_]+]] = OpIAdd %uint [[val1]] [[val2]]
  // CHECK-NEXT: [[target_ptr:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_uint %output %int_0 %uint_0
  // CHECK-NEXT: OpStore [[target_ptr]] [[add_res]]
  x.a = input;
  x.b = output;
  output[0] = x.a[1] + x.b[2];
}
