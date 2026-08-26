// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
RWTexture2D<float4> uav1 : register(u3);

float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  float4 r = 0;
  uint status;
  r += uav1[b];
  r += uav1.Load(a);
  uav1.Load(a, status);
  if (CheckAccessFullyMapped(status)) {
    r += 3;
  }
  uav1.Load(a, status);
  if (CheckAccessFullyMapped(status)) {
    r += 3;
  }
  uav1[b] = r;
  return r;
}
