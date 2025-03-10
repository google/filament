// RUN: %dxc -T cs_6_0 -E main -spirv -fcgl  %s -spirv | FileCheck %s


// CHECK: %src_main = OpFunction %void
// CHECK: OpFunctionCall %void %S_operator_Call %s
// CHECK: OpFunctionEnd
// CHECK: %S_operator_Call = OpFunction %void None
// CHECK-NEXT: OpFunctionParameter
// CHECK-NEXT: OpLabel
// CHECK-NEXT: OpReturn
// CHECK-NEXT: OpFunctionEnd


struct S
{
    void operator()();
};

void S::operator()()
{
}

[numthreads(8,8,1)]
void main(uint32_t3 gl_GlobalInvocationID : SV_DispatchThreadID)
{
  S s;
  s();
}
 
