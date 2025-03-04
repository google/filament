// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// CHECK: Offsets to texture access operations must be immediate values

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);


int x;
int y;

float4 main(float2 a : A) : SV_Target {
  float4 r = 0;
  r = text1.Sample(samp1, a, int2(x+y,x-y));

  return r;
}
