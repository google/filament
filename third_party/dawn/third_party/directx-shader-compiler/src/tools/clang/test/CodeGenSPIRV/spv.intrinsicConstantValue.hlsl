// RUN: %dxc -Od -T cs_6_8 -spirv -fcgl %s | FileCheck %s

// CHECK: %spirvIntrinsicType = OpTypeInt 8 0
using uint8_t [[vk::ext_capability(/* Int8 */ 39)]] =
    vk::SpirvType</* OpTypeInt */ 21, 8, 8, vk::Literal<vk::integral_constant<uint, 8> >,
                  vk::Literal<vk::integral_constant<bool, false> > >;

[[vk::ext_instruction(/* OpConstant */ 43)]] uint8_t mkconsant([[vk::ext_literal]] int v);

// CHECK: OpConstant %spirvIntrinsicType 42
static const uint8_t K = mkconsant(42);

[numthreads(1, 1, 1)] void main() {}
