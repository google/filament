// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpName %type_PushConstant_ "type.PushConstant."
// CHECK: OpMemberName %type_PushConstant_ 0 "a"
// CHECK: OpMemberName %type_PushConstant_ 1 "b"
// CHECK: OpMemberName %type_PushConstant_ 2 "c"

// CHECK: %type_PushConstant_ = OpTypeStruct %int %float %v3float
// CHECK: %_ptr_PushConstant_type_PushConstant_ = OpTypePointer PushConstant %type_PushConstant_
[[vk::push_constant]]
struct {
    int    a;
    float  b;
    float3 c;
}
// CHECK: %PushConstants = OpVariable %_ptr_PushConstant_type_PushConstant_ PushConstant
PushConstants;

RWBuffer<int> Output;

[numthreads(1, 1, 1)]
void main() {
// CHECK: OpAccessChain %_ptr_PushConstant_int %PushConstants %int_0
    Output[0] = PushConstants.a;
}
