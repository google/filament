// RUN: %dxilver 1.4 | %dxc -E main -T ps_6_0 -validator-version 1.4 %s | %listparts | FileCheck %s -check-prefix=NODEDUP
// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_0 -validator-version 1.7 %s | %listparts | FileCheck %s -check-prefix=DEDUP

// NODEDUP: #1 - ISG1 (124 bytes)
// DEDUP: #1 - ISG1 (116 bytes)

float4 main(float4 f0 : TEXCOORD0, uint2 u1 : U1, float2 f2 : TEXCOORD2) : SV_Target
{
	return f0 + (u1 * f2).xyxy;
}
