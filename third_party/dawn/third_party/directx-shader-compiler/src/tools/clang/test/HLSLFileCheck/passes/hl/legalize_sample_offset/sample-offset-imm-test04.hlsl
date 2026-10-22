// RUN: %dxc /O0 /Od /Tps_6_0 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: entry
Texture2D g_Tex;
SamplerState g_Sampler;
void unused() { }
float4 main(float4 pos : SV_Position, float4 user : USER, bool b : B) : SV_Target {
	unused();
	int2 offset[2] = { int2(0,1), int2(1,0) };
	if (b) user = g_Tex.SampleLevel(g_Sampler, pos.xy, 0.0, int2(offset[1].x, offset[0].y));
	return user * pos;
}