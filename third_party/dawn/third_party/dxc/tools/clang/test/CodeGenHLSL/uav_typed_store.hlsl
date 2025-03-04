// RUN: %dxc -E main -T ps_6_0 %s

RWTexture1D<float4> uav1 : register(u3);
RWBuffer<float4> uav2;

float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  uav1[0] = 2;
  uav2[1] = 3;
  return 0;
}
