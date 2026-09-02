// RUN: %dxc -O3 -T lib_6_8 -HV 2021 -spirv -fcgl -fspv-target-env=universal1.5 %s | FileCheck %s

static const uint32_t kIterCnt = 1024;
static const uint32_t kSizeShared = 4096;
static const uint32_t kOffset = 2048;

groupshared uint32_t sharedMem[kSizeShared * sizeof(uint32_t)];

export void testcase(uint32_t threadIdInGroup, uint32_t threadGroupSize) {
  // CHECK: OpULessThan %bool %{{.*}} %uint_1024
  for (uint32_t i = threadIdInGroup; i < kIterCnt; i += threadGroupSize) {
    // CHECK: [[INDEX:%.*]] = OpIAdd %uint %uint_2048
    // CHECK: OpAccessChain %_ptr_Workgroup_uint %sharedMem [[INDEX]]
    sharedMem[kOffset + i] = 0xffu;
  }
}
