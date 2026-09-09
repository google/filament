// RUN: %dxc /T ps_6_0 /E main %s | FileCheck %s

// Starting with SM 5.1, fxc ignores unused resources when allocating registers for used resources

// FXC - Tex1 texture float4 2d T0 t0 1
// CHECK: Tex1 texture f32 2d T0 t0 1

Texture2D Tex0 : register(t0);
Texture2D Tex1;
float4 main() : SV_Target
{
	return Tex1.Load((uint3)0);
}