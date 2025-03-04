// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s
// CHECK: @dx.op.textureLoad.f32(i32 66
// CHECK: call void @dx.op.textureStore.f32(i32 67
// CHECK-NOT: @dx.op.textureLoad.f32(i32 66
// CHECK-NOT: call void @dx.op.textureStore.f32(i32 67

struct PS_INPUT
{
 float4 pos : SV_POSITION;
 float2 vPos : TEXCOORD0;
 float2 sPos : TEXCOORD1;
};

RWTexture2D<float4> t_output : register(u0);

float4 psMain(in PS_INPUT I)
{
 return float4(0, 0, 0, 1);
}


[numthreads(8, 8, 1)]
void main(uint2 dtid : SV_DispatchThreadID)
{
 PS_INPUT I = {
 float4(0,0, 0, 1),
 float2(dtid),
 float2(dtid)
 };

 uint2 uspos = uint2(I.pos.xy);

 {
 uint w, h;
 t_output.GetDimensions(w, h);
 if (any(uspos >= uint2(w, h)))
 return;
 }

 float3 prev = t_output[uspos].rgb;
 t_output[uspos].rgb = float3(psMain(I).rgb + prev);
}

