// RUN: %dxc -E main -T vs_6_0 %s| FileCheck %s

// CHECK: Not all elements of SV_Position were written

float4 main(float4 a : A, out float4 pos: SV_POSITION ) : COLOR
{
  return 2.3;
}