// RUN: %dxc /T ps_6_0 /E main -flegacy-resource-reservation %s | FileCheck %s

// Until SM 5.0, fxc takes into account explicit register allocation of unused resources
// when allocating registers for used resources.

// FXC - Tex1[1] texture float4 2d t4 1
// CHECK: Tex1 texture f32 2d T0 t3 2

Texture2D Tex0[2] : register(t1);
Texture2D Tex1[2];
float4 main() : SV_Target
{
	return Tex1[1].Load((uint3)0);
}