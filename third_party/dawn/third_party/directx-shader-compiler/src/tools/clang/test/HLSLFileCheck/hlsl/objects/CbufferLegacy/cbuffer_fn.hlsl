// RUN: %dxc -E VSMain -T vs_6_0 %s | FileCheck %s

// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(

cbuffer C {
  float4 f4;
  float4 get_f4() { return f4; }
};

struct PSInput {
 float4 position : SV_POSITION;
 float4 color : COLOR;
};

PSInput VSMain(float4 position: POSITION, float4 color: COLOR) {
 float aspect = 320.0 / 200.0;
 PSInput result;
 result.position = position;
 result.position.y *= aspect;
 result.color = color * get_f4();
 return result;
}
