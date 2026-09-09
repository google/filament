// RUN: %dxc -T vs_6_0 -E main -Wno-dx9-deprecation -verify %s

float4 main(float4 input : SV_Position) : POSITION { /* expected-no-diagnostics */
  return input;
}
