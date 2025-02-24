// RUN: %dxc -T vs_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpMemberDecorate %type__Globals 0 Offset 0
// CHECK: OpMemberDecorate %type__Globals 1 Offset 16
// CHECK: OpMemberDecorate %type__Globals 2 Offset 32
// CHECK: OpMemberDecorate %type__Globals 3 Offset 64
// CHECK: OpMemberDecorate %type__Globals 4 Offset 96
// CHECK: OpMemberDecorate %type__Globals 5 Offset 144
// CHECK: OpMemberDecorate %type__Globals 6 Offset 160
// CHECK: OpMemberDecorate %type__Globals 7 Offset 192
// CHECK: OpMemberDecorate %type__Globals 8 Offset 240
// CHECK: OpMemberDecorate %type__Globals 9 Offset 288
// CHECK: OpMemberDecorate %type__Globals 10 Offset 336

float  x       : register(c0);  // Offset:   0   Size:  4
float  y       : register(c1);  // Offset:  16   Size:  4
float  z       : register(c2);  // Offset:  32   Size:  4
float  w       : register(c4);  // Offset:  64   Size:  4
float2 xy      : register(c6);  // Offset:  96   Size:  8
float3 xyz     : register(c9);  // Offset: 144   Size: 12
float4 xyzw    : register(c10); // Offset: 160   Size: 16
float4 arr4[3] : register(c12); // Offset: 192   Size: 48
float2 arr2[3] : register(c15); // Offset: 240   Size: 40
float3 arr3[3] : register(c18); // Offset: 288   Size: 44
float  s       : register(c21); // Offset: 336   Size:  4

float4 main(float4 Pos : Position) : SV_Position
{
  float4 output = Pos;
  output.x    += x + s;
  output.y    += y;
  output.z    += z;
  output.w    += w;
  output.xy   += xy + arr2[0];
  output.xyz  += xyz + arr3[1];
  output.xyzw += xyzw + arr4[2];
  return output;
}
