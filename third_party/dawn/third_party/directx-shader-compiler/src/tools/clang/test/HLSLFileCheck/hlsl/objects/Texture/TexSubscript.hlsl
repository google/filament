// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure use 0 as mip level.
// CHECK:i32 0, i32 39
// CHECK:i32 0, i32 38, i32 38

Texture1D<float4> tex;
Texture2D<float4> tex2;

float4 main() : SV_Target
{
	return  tex[39] + tex2[uint2(38,38)];
}