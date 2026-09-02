// RUN: %dxc -T cs_6_8 -HV 2021 -O0 -spirv -fspv-target-env=universal1.5 %s | FileCheck %s

// CHECK: [[Int8Type:%.*]] = OpTypeInt 8 0
using Int8Type = vk::SpirvType</* OpTypeInt */ 21, 8, 8,
                               vk::Literal<vk::integral_constant<uint32_t, 8> >,
                               vk::Literal<vk::integral_constant<bool, 0> > >;

// CHECK: [[MatrixType:%.*]] = OpTypeCooperativeMatrixKHR [[Int8Type]] %uint_3 %uint_16 %uint_16 %uint_0
using I8MatA = vk::SpirvOpaqueType<
    /* OpTypeCooperativeMatrixKHR */ 4456, Int8Type,
    vk::integral_constant<uint, /* ScopeSubgroup */ 3>,
    vk::integral_constant<uint, 16>, vk::integral_constant<uint, 16>,
    vk::integral_constant<uint, /* Use */ 0> >;

template <typename ResultType, typename PointerType>
[[vk::ext_instruction(/* OpCooperativeMatrixLoadKHR */ 4457)]] ResultType
__builtin_spv_CooperativeMatrixLoadKHR([[vk::ext_reference]] PointerType pointer,
    uint32_t memory_layout, uint32_t stride, [[vk::ext_literal]] uint32_t memory_operand);

StructuredBuffer<uint32_t> buffer : register(t0, space0);

[numthreads(32, 1, 1)] void main() {
  [[vk::ext_extension("SPV_KHR_cooperative_matrix")]]
  [[vk::ext_capability(/* CooperativeMatrixKHRCapability */ 6022)]]
  [[vk::ext_capability(/* VulkanMemoryModel */ 5345)]]
  [[vk::ext_capability(/* Int8 */ 39)]]
  // CHECK: OpCooperativeMatrixLoadKHR [[MatrixType]] %{{.*}} %uint_0 %uint_32 None
  I8MatA matA = __builtin_spv_CooperativeMatrixLoadKHR<I8MatA>(buffer[0], /* rowMajor */ 0, 32, 0);
}
