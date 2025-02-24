// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure array of struct with resource works.

//CHECK: x.1.s                             sampler      NA          NA      S0             s4     1
//CHECK: x.0.s                             sampler      NA          NA      S1             s3     1
//CHECK: x.1.x                             texture     f32          2d      T0             t5     1
//CHECK: x.0.y                             texture     f32          2d      T1             t4     1
//CHECK: m                                 texture     f32          2d      T2             t9     1

struct X {
   Texture2D x;
   SamplerState s ;
   Texture2D y;
};

X x[2] : register(t3) : register(s3);

cbuffer A {
  Texture2D m : register(t9);
}

float4 main(float2 uv:UV, uint i:I) : SV_Target
{
	return x[1].x.Sample(x[1].s,uv) + x[0].y.Sample(x[0].s, uv) + m.Sample(x[0].s, uv);
}