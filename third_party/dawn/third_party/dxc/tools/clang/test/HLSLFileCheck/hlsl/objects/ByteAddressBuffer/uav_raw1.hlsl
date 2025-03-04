// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: bufferLoad
// CHECK: bufferLoad
// CHECK: bufferLoad
// CHECK: bufferLoad
// CHECK: bufferLoad
// CHECK: bufferStore
// CHECK: bufferStore
// CHECK: bufferStore
// CHECK: bufferStore

RWByteAddressBuffer uav1 : register(u3);

float4 main(uint4 a : A, uint4 b : B) : SV_Target
{
  uint4 r = 0;
  uint status;
  r += uav1.Load(a.x);
  r += uav1.Load(a.y, status); r += status;
  r.xy += uav1.Load2(a.z, status); r += status;
  r.xyz += uav1.Load3(a.w);
  r += uav1.Load4(b.x);
  uav1.Store(b.x, r.x);
  uav1.Store2(b.y, r.xy);
  uav1.Store3(b.z, r.xyz);
  uav1.Store4(b.w, r);
  return r;
}
