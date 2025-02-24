// RUN: %dxc -E main -T vs_6_3 %s | FileCheck %s

// TODO: fix consistency of error between invalid for shader model vs. invalid location
// CHECK: error: Semantic SV_ShadingRate is invalid for shader model: vs

struct VS_OUT {
  float4 pos : SV_Position;
  uint rate : SV_ShadingRate;
};

VS_OUT main(float4 pos : Pos, uint rate : ShadingRate) {
  VS_OUT Out;
  Out.pos = pos;
  Out.rate = rate;
  return Out;
}
