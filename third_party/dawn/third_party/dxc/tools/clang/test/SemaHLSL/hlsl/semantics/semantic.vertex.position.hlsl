// RUN: %dxc -T vs_6_0 -E main -verify %s

float4 main(float4 input : SV_Position) : POSITION { /* expected-warning{{'POSITION' is a user-defined semantic; did you mean 'SV_Position'?}} */
  return input;
}
