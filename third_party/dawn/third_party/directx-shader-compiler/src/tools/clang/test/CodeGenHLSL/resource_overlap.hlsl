// RUN: %dxc -E main -T ps_6_0 %s

RWTexture1D<float4> uav1 : register(u3);
RWBuffer<float4> uav2;

Texture1D<float4> srv1;
Buffer<float4> srv2;

cbuffer A {
  float a;
};

cbuffer B {
  float b;
};

SamplerState s0;
SamplerState s1;

float4 main() : SV_Target
{
  uav1[0] = srv1.Sample(s0, 1) + srv1.Sample(s1,1) + a;
  uav2[1] = srv2[1] + b;
  return 0;
}
