// RUN: %dxc -T ps_6_0 -E main -spirv -fcgl  %s -spirv | FileCheck %s

[[vk::ext_type_def(0, 21)]]
void createTypeInt([[vk::ext_literal]] int sizeInBits,
                   [[vk::ext_literal]] int signedness);

[[vk::ext_type_def(1,  23)]]
void createTypeVector([[vk::ext_reference]] vk::ext_type<0> typeInt,
                      [[vk::ext_literal]] int componentCount);

//CHECK: %spirvIntrinsicType = OpTypeInt 32 0
//CHECK: %spirvIntrinsicType_0 = OpTypeVector %spirvIntrinsicType 4

vk::ext_type<0> foo1;
vk::ext_type<1> foo2;
float main() : SV_Target
{
    createTypeInt(32, 0);
    createTypeVector(foo1, 4);
    return 0.0;
}
