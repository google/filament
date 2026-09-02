// RUN: %dxc -T lib_6_3 -Od %s | FileCheck %s

// Verifies packoffset of struct with multiple levels of inheritance
// CHECK:       } cb_pld;                                     ; Offset:   32
// CHECK:       float f;                                      ; Offset:    4
// CHECK:   } CB;                                             ; Offset:    0 Size:    72

// Make sure bitcast to base for `this` as part of the method call gets folded into GEP on -Od
// CHECK: define void [[raygen1:@"\\01\?raygen1@[^\"]+"]]() #0 {
// CHECK-NOT: bitcast
// CHECK:   ret void

struct SuperBasePayload {
  float g;
  bool DoInner() { return g < 0; }
};

struct BasePayload : SuperBasePayload {
  float f;
  bool DoSomething() { return f < 0 && DoInner(); }
};

struct MyPayload : BasePayload {
  int i;
  float4 color;
  uint2 pos;
};

RaytracingAccelerationStructure RTAS : register(t5);
RWByteAddressBuffer BAB :  register(u0) ;

cbuffer CB : register(b0) {
  MyPayload cb_pld : packoffset(c2.x);
  float f : packoffset(c0.y);
}

[shader("raygeneration")]
void raygen1()
{
  MyPayload p = cb_pld;
  p.pos = DispatchRaysIndex();
  float3 origin = {0, 0, 0};
  float3 dir = normalize(float3(p.pos / (float)DispatchRaysDimensions(), 1));
  RayDesc ray = { origin, 0.125, dir, 128.0};
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, ray, p);
  BAB.Store(p.pos.x + p.pos.y * DispatchRaysDimensions().x, p.DoSomething());
}
