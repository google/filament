// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct StructA
{
    float3 one;
    float3 two;
};

// CHECK: %type_ConstantBuffer_StructA = OpTypeStruct %v3float %v3float
// CHECK: %PushConstants = OpVariable %_ptr_PushConstant_type_ConstantBuffer_StructA PushConstant
[[vk::push_constant]] ConstantBuffer<StructA> PushConstants;

void main()
{
}
