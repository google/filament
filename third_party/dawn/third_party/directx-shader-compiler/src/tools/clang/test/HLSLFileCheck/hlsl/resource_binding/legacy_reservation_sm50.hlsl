// RUN: %dxc /T ps_6_0 /E main -flegacy-resource-reservation %s | FileCheck %s

// Until SM 5.0, FXC takes into account explicit register allocation of unused resources
// when allocating registers for used resources.

// FXC - Tex1 texture float4 2d t1 1
// CHECK: Tex1 texture f32 2d T0 t1 1

Texture2D Tex0 : register(t0);
Texture2D Tex1;
float4 main() : SV_Target
{
	return Tex1.Load((uint3)0);
}