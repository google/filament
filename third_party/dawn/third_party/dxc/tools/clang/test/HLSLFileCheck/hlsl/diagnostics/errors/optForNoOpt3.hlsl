// RUN: %dxc -Zi -E main -Od -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_DB
// RUN: %dxc -E main -Od -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NODB

// CHK_DB: 16:7: error: Offsets to texture access operations must be immediate values
// CHK_NODB: Offsets to texture access operations must be immediate values.

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);


int x;
int y;

float4 main(float2 a : A) : SV_Target {
  float4 r = 0;
  r = text1.Sample(samp1, a, int2(x+y,x-y));

  return r;
}
