// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: define void [[raygen1:@"\\01\?raygen1@[^\"]+"]]() #0 {
// CHECK: fdiv float
// CHECK: fdiv float
// CHECK: fmul float
// CHECK: fmul float
// CHECK: fmul float
// CHECK: fmul float
// CHECK: fmul float
// CHECK: ret void

float3 PreciseMul(float3 a, float3 b) {
  precise float3 result = a * b;
  return result;
}

struct MyPayload {
  float4 color;
  uint2 pos;
  void Modify(float3 v) {
    color.xyz = PreciseMul(v, color.xyz);
  }
};

#define WIDTH 16
RWStructuredBuffer<MyPayload> SB : register(u7);

struct Data {
  MyPayload pld;
  RayDesc ray;
  void Write() {
    SB[pld.pos.x + pld.pos.y * WIDTH] = pld;
  }
};

void DoIt(inout Data data) {
  data.pld.Modify(data.ray.Direction);
}

RaytracingAccelerationStructure RTAS : register(t5);

[shader("raygeneration")]
void raygen1()
{
  Data data = (Data)0;
  data.pld.pos = DispatchRaysIndex().xy;
  data.ray.Origin = float3(0, 0, 0);
  data.ray.Direction = normalize(float3(data.pld.pos / (float2)DispatchRaysDimensions().xy, 1));
  data.ray.TMin = 0.125;
  data.ray.TMax = 128.0;
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, data.ray, data.pld);
  DoIt(data);
  data.Write();
}
