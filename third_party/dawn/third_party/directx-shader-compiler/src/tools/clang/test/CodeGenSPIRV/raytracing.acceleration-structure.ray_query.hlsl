// RUN: %dxc -E main -T vs_6_5 -fspv-target-env=vulkan1.2 -fspv-extension=SPV_KHR_ray_query %s -spirv | FileCheck %s

// CHECK-NOT: OpCapability RayTracingKHR
// CHECK: OpCapability RayQueryKHR
// CHECK: OpCapability Shader
// CHECK-NEXT: OpExtension "SPV_KHR_ray_query"
// CHECK-NOT: OpExtension "SPV_KHR_ray_tracing"

RaytracingAccelerationStructure test_bvh;

float main(RayDesc rayDesc : RAYDESC) : OUT {
  RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery1;
  rayQuery1.TraceRayInline(test_bvh, 0, 1, rayDesc);
  return 0;
}
