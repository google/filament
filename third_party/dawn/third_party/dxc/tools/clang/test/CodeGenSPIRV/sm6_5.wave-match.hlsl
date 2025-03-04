// RUN: %dxc -E main -T ps_6_5 -spirv -O0 -fspv-target-env=vulkan1.1 %s | FileCheck %s
// RUN: not %dxc -E main -T ps_6_5 -spirv -O0 %s 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR

// CHECK-ERROR: error: Vulkan 1.1 is required for Wave Operation but not permitted to use

// CHECK: OpCapability GroupNonUniformPartitionedNV
// CHECK: OpExtension "SPV_NV_shader_subgroup_partitioned"

uint4 main(uint4 input : ATTR0) : SV_Target {
// CHECK: [[input:%[0-9]+]] = OpLoad %v4uint %input
// CHECK:       {{%[0-9]+}} = OpGroupNonUniformPartitionNV %v4uint [[input]]
    uint4 res = WaveMatch(input);
    return res;
}
