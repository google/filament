// RUN: %dxc /O3 /Tps_6_0 /Emain  %s | FileCheck %s

// Make sure only one cbuffer is emitted for the final
// dxil.

// CHECK-LABEL: ; Buffer Definitions:
// CHECK-NOT: CB1            cb1

cbuffer BAR {
  float bar;
}
cbuffer FOO {
  float foo;
}

Texture2D tex0;
SamplerState samp0;

float main(float2 a : A) : SV_Target {
  tex0.Sample(samp0, a+float2(bar,-bar));
  return foo;
}
