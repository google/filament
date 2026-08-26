// RUN: %dxc /O0 /Tps_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry
Texture2D g_Tex;
SamplerState g_Sampler;
void unused() { }
float4 main(float4 pos : SV_Position, float4 user : USER, bool b : B) : SV_Target {
	unused();
	int2 offset = int2(0,-1);
	float4 g_Buffer = {-1.0, 1.0, -2.0, 2.0};
	float4 shift = float4(g_Buffer[offset.x], g_Buffer[offset.x], g_Buffer[offset.x], g_Buffer[offset.y]);
	if (b) user = g_Tex.SampleLevel(g_Sampler, pos.xy, 0.0, offset.xy);
	return user * (pos * shift);
}