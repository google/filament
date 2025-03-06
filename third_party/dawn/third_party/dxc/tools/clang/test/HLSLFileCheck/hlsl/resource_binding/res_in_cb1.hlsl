// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure register binding on struct works.
//CHECK: tx0.s                             sampler      NA          NA      S0             s0     1
//CHECK: tx1.s                             sampler      NA          NA      S1             s1     1
//CHECK: s                                 sampler      NA          NA      S2             s3     1
//CHECK: tx0.t2                            texture     f32          2d      T0             t1     1
//CHECK: tx0.t                             texture     f32          2d      T1             t0     1
//CHECK: tx1.t2                            texture     f32          2d      T2             t6     1
//CHECK: tx1.t                             texture     f32          2d      T3             t5     1
//CHECK: x                                 texture     f32          2d      T4             t3     1

struct LegacyTex
{
  Texture2D t;
  Texture2D t2;
  SamplerState  s;
};

LegacyTex tx0 : register(t0) : register(s0);
LegacyTex tx1 : register(t5) : register(s1);

float4 tex2D(LegacyTex tx, float2 uv)
{
  return tx.t.Sample(tx.s,uv) + tx.t2.Sample(tx.s, uv);
}

cbuffer n {
  Texture2D x: register(t3);
  SamplerState s : register(s3);
}

float4 main(float2 uv:UV) : SV_Target
{
  return tex2D(tx0,uv) + tex2D(tx1,uv) + x.Sample(s,uv);
}
