// RUN: %dxc -E main -T vs_6_4 %s | FileCheck %s

// CHECK:       ; Note: shader requires additional functionality:
// CHECK-NEXT:  ;       Shading Rate
// CHECK:       ; Input signature:
// CHECK:       ; Output signature:
// CHECK:       ; SV_ShadingRate           0   x           1SHDINGRATE    uint
// CHECK:       ; Input signature:
// CHECK:       ; Output signature:
// CHECK:       ; SV_ShadingRate           0        nointerpolation
// CHECK: !{i32 1, !"SV_ShadingRate", i8 5, i8 29,

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
