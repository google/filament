// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: There can only be up to two input attributes of SV_Barycentrics with different perspective interpolation mode.

float4 main(float3 bary : SV_Barycentrics, noperspective float3 bary1 : SV_Barycentrics1, float3 bary2 : SV_Barycentrics2) : SV_Target
{
  return 1;
}