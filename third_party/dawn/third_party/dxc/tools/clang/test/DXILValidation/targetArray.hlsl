// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

uint i;
float4 main(int4 a : A, out float4 o[2] : SV_TARGET, out float4 o2[2][2] : SV_TARGET2) : SV_TARGET7
{
  o[0] = a+2;
  o[1] = a+3;
  o[i] = a+10;
  o2[0][0] = a+5;
  o2[0][1] = a+6;
  o2[1][0] = a+7;
  o2[1][1] = a+8;
  o2[i][i] = a + 12;
  return a + 16;
}