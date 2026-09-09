// RUN: %dxc -T ps_6_0 -DIDX=[]  -flegacy-resource-reservation %s | FileCheck %s
// RUN: %dxc -T ps_6_0 -DIDX=[][3] -flegacy-resource-reservation %s | FileCheck %s

// CHECK: error: unbounded resources are not supported with -flegacy-resource-reservation

Texture2D Tex IDX;
float4 main() : SV_Target
{
	return 0;
}