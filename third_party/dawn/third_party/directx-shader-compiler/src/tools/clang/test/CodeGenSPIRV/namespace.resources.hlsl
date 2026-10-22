// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %rw1 DescriptorSet 0
// CHECK: OpDecorate %rw1 Binding 0
// CHECK: OpDecorate %rw2 DescriptorSet 0
// CHECK: OpDecorate %rw2 Binding 1
// CHECK: OpDecorate %rw3 DescriptorSet 0
// CHECK: OpDecorate %rw3 Binding 2

// CHECK: OpMemberDecorate %type_RWStructuredBuffer_v4float 0 Offset 0
// CHECK: OpDecorate %type_RWStructuredBuffer_v4float BufferBlock

// CHECK: OpMemberDecorate %type__Globals 0 Offset 0
// CHECK: OpDecorate %type__Globals Block

RWStructuredBuffer<float4> rw1;

namespace A {
  RWStructuredBuffer<float4> rw2;
  
  namespace B {
    RWStructuredBuffer<float4> rw3;
  }  // end namespace B

}  // end namespace A

// Check that resources are not added to the globals struct.
// CHECK: %type__Globals = OpTypeStruct %int
int c;

float4 main(float4 PosCS : SV_Position) : SV_Target
{
// CHECK: OpAccessChain %_ptr_Uniform_v4float %rw1 %int_0 %uint_0
// CHECK: OpAccessChain %_ptr_Uniform_v4float %rw2 %int_0 %uint_1
// CHECK: OpAccessChain %_ptr_Uniform_v4float %rw3 %int_0 %uint_2
  return rw1[0] + A::rw2[1] + A::B::rw3[2];
}
