// RUN: %dxc -E main -T cs_6_4 -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability RayQueryKHR
// CHECK: OpExtension "SPV_KHR_ray_query"

// CHECK: %accelerationStructureNV = OpTypeAccelerationStructureKHR

RaytracingAccelerationStructure test_bvh;

[numthreads(1, 1, 1)]
void main() {}
