// RUN: %dxc -E main -T cs_6_0 /Gec -HV 2016 %s | FileCheck %s

// CHECK: define void @main()
// CHECK: ret void

Texture2D<float3> InColor : register(t0);
RWTexture2D<float3> Color : register(u0);
RWTexture2D<float3> OutColor : register(u1);
groupshared uint PixelCountH;

[numthreads(64,16,1)]
void main( uint3 gtid : SV_GroupThreadID )
{
 uint2 a = gtid.xy;
 Color[a] = InColor[a];
 PixelCountH = Color[a].x * 1;
 OutColor[a] = PixelCountH;
}
