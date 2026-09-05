// RUN: %dxc -E VSMain -T vs_6_0 %s | FileCheck %s

// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(

struct S {
  float4 f4;
  float4 get_f4() { return f4; }
};

cbuffer C {
  S s1;
  S s2;
};

struct PSInput {
 float4 position : SV_POSITION;
 float4 color : COLOR;
};

PSInput VSMain(float4 position: POSITION, float4 color: COLOR) {
 float aspect = 320.0 / 200.0;
 PSInput result;
 S s3 = s2;
 result.position = s1.get_f4();
 result.color = s2.get_f4() * s3.get_f4();
 return result;
}
