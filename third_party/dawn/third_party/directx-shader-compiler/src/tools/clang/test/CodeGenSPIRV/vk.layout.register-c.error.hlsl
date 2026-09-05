// RUN: not %dxc -T vs_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 15:18: error: found offset overlap when processing register(c8) assignment
// CHECK: 16:18: error: found offset overlap when processing register(c9) assignment
// CHECK: 17:18: error: found offset overlap when processing register(c10) assignment

float  x       : register(c0);
float  y       : register(c1);
float  z       : register(c2);
float  w       : register(c3);
float2 xy      : register(c4);
float3 xyz     : register(c5);
float4 xyzw    : register(c6);
float4 arr4[3] : register(c7);
float2 arr2[3] : register(c8);   // This should generate an overlap error with the previous line
float3 arr3[3] : register(c9);   // This should generate an overlap error with the previous line
float  s       : register(c10);  // This should generate an overlap error with the previous line

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
