// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: call i64 @dx.op.waveActiveOp.i64(i32 119, i64 8, i8 0, i8 0)

struct MyPayload {
  float4 color;
  uint2 pos;
};

RaytracingAccelerationStructure RTAS : register(t5);

RWByteAddressBuffer Log;

[shader("raygeneration")]
void raygen1()
{
  MyPayload p = (MyPayload)0;
  p.pos = DispatchRaysIndex().xy;

  uint offset = 0;
  Log.InterlockedAdd(0, 8, offset);
  offset += WaveActiveSum(8);
  Log.Store(offset, p.pos.x);
  Log.Store(offset + 4, p.pos.y);

  float3 origin = {0, 0, 0};
  float3 dir = normalize(float3(p.pos / (float)DispatchRaysDimensions(), 1));
  RayDesc ray = { origin, 0.125, dir, 128.0};
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, ray, p);
}
