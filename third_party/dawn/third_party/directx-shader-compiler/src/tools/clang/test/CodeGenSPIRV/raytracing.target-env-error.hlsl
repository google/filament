// RUN: not %dxc -T lib_6_3 -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: error: Vulkan 1.1 with SPIR-V 1.4 is required for Raytracing but not permitted to use

struct Payload {
  float4 color;
};
struct CallData {
  float4 data;
};
struct Attribute {
  float2 bary;
};

RaytracingAccelerationStructure rs;
[shader("closesthit")]
void main(inout Payload MyPayload, in Attribute MyAttr) {
  Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
  CallData myCallData = { float4(0.0f,0.0f,0.0f,0.0f) };
  RayDesc rayDesc;
  rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
  rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
  rayDesc.TMin = 0.0f;
  rayDesc.TMax = 1000.0f;
  TraceRay(rs, 0x0, 0xff, 0, 1, 0, rayDesc, myPayload);
  CallShader(0, myCallData);
}
