// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure report error when dynamic indexing on resource array inside cbuffer.
//CHECK:Index for resource array inside cbuffer must be a literal expression

struct X {
   Texture2D x;
   SamplerState s ;
   Texture2D y;
};

X x[2] : register(t3) : register(s3);

float4 main(float2 uv:UV, uint i:I) : SV_Target
{
	return x[1].x.Sample(x[1].s,uv) + x[0].y.Sample(x[i].s, uv);
}