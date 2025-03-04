// RUN: %dxc -E main -T vs_6_4 %s | FileCheck %s

// CHECK: error: invalid semantic 'SV_ShadingRate'

struct VS_OUT {
  float4 pos : SV_Position;
  uint rate : ShadingRate;
};

VS_OUT main(float4 pos : Pos, uint rate : SV_ShadingRate) {
  VS_OUT Out;
  Out.pos = pos;
  Out.rate = rate;
  return Out;
}
