// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpMemberName %type__Globals 0 "a"
// CHECK: OpMemberName %type__Globals 1 "b"
// CHECK: OpMemberName %type__Globals 2 "c"
// CHECK: OpName %_Globals "$Globals"

// CHECK: OpDecorate %_Globals DescriptorSet 0
// CHECK: OpDecorate %_Globals Binding 0

// CHECK: %type__Globals = OpTypeStruct %int %int %int

namespace A {
  int a;

  namespace B {
    int b;
  }  // end namespace B

}  // end namespace A

int c;

float4 main(float4 PosCS : SV_Position) : SV_Target
{
// CHECK: OpAccessChain %_ptr_Uniform_int %_Globals %int_1
// CHECK: OpAccessChain %_ptr_Uniform_int %_Globals %int_0
// CHECK: OpAccessChain %_ptr_Uniform_int %_Globals %int_2
  int newInt = A::B::b + A::a + c;
  return float4(0,0,0,0);
}
