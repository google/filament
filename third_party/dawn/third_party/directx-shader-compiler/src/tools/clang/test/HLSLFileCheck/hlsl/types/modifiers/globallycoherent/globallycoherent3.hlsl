// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: 'globallycoherent' is not a valid modifier for a declaration of type 'float'

globallycoherent RWTexture1D<float4> uav1 : register(u3);
RWBuffer<float4> uav2;
globallycoherent float m;
float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  globallycoherent  RWTexture1D<float4> uav3 = uav1;
  globallycoherent float x = 3;
  uav3[0] = 0;
  uav1[0] = 2;
  uav2[1] = 3;
  return 0;
}
