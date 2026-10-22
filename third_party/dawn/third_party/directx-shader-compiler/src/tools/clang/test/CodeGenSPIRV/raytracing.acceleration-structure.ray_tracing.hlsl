// RUN: %dxc -E main -T lib_6_6 -fspv-target-env=vulkan1.2 -fspv-extension=SPV_KHR_ray_tracing  %s -spirv | FileCheck %s

// CHECK-NOT: OpCapability RayQueryKHR
// CHECK: OpCapability RayTracingKHR
// CHECK-NEXT: OpExtension "SPV_KHR_ray_tracing"
// CHECK-NOT: OpExtension "SPV_KHR_ray_query"

RaytracingAccelerationStructure rs;

struct Payload { float4 color; };
struct Attribute { float2 bary; };

[shader("closesthit")]
void main(inout Payload MyPayload, in Attribute MyAttr) {

  Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
  rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
  rayDesc.TMin = 0.0f;
  rayDesc.TMax = 1000.0f;

  TraceRay(rs, 0x0, 0xff, 0, 1, 0, rayDesc, myPayload);
}
