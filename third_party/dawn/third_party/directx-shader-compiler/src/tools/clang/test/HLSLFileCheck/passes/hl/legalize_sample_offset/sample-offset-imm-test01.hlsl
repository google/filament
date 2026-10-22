// RUN: %dxc /O0 /Tps_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry
Texture2D g_Tex;
SamplerState g_Sampler;
void unused() { }
float4 main(float4 pos : SV_Position, float4 user : USER, bool b : B) : SV_Target {
	unused();
	int2 offset = int2(0,-1);
	if (b) user = g_Tex.SampleLevel(g_Sampler, pos.xy, 0.0, offset.xy);
	return user * pos;
}