// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float g1[4], g2[8];
int g3[4], g4[8];

float main(float4 a : A, int b : B, int c : C) : SV_TARGET
{
  float x1[4];
  int x2[2];
  x1[0] = g1[b];
  x1[1] = g2[b];
  x1[2] = g1[b+2];
  x1[3] = g1[b+3];

  x2[0] = g3[b+11] + g4[c+13];
  x2[1] = g4[c+13] - 13;

  return x1[ x2[c] ];
}
