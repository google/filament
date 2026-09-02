// RUN: %dxc -T cs_6_0 %s -E main -spirv -fspv-target-env=vulkan1.1 | FileCheck %s

// CHECK-NOT: OpCapability GroupNonUniformPartitionedNV
// CHECK-NOT: OpExtension "SPV_NV_shader_subgroup_partitioned"
// CHECK:     OpCapability GroupNonUniformArithmetic

RWBuffer<int> value;

[numthreads(4, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID) {
  uint sum = 0;
  switch (value[threadID.x]) {
    case 0:
// CHECK: OpGroupNonUniformIAdd {{.*}} {{.*}} Reduce {{.*}}
      sum += WaveActiveSum(1);
    default:
// CHECK: OpGroupNonUniformIAdd {{.*}} {{.*}} Reduce {{.*}}
      sum += WaveActiveSum(10);
      break;
  }
  value[threadID.x] = sum;
}
