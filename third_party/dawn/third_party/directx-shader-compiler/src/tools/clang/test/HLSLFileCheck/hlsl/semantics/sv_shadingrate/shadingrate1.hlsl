// RUN: %dxc -E main -T ps_6_4 %s | FileCheck %s

// CHECK:       ; Note: shader requires additional functionality:
// CHECK-NEXT:  ;       Shading Rate
// CHECK:       ; Input signature:
// CHECK:       ; SV_ShadingRate           0   x           1SHDINGRATE    uint
// CHECK:       ; Output signature:
// CHECK:       ; Input signature:
// CHECK:       ; SV_ShadingRate           0        nointerpolation
// CHECK:       ; Output signature:
// CHECK: !{i32 1, !"SV_ShadingRate", i8 5, i8 29,

float2 main(float2 v0 : Texcoord0, uint rate : SV_ShadingRate) : SV_Target {
  uint2 rate2 = uint2(1 << (rate & 0x3), 1 << ((rate >> 2) & 0x3));
  return v0 * rate2;
}
