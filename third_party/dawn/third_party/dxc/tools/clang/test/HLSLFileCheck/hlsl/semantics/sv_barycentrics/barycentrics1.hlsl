// RUN: %dxc -E main -T ps_6_1 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_1 %s | FileCheck %s -check-prefixes=CHECK,CHK16

// CHK16: Note: shader requires additional functionality:
// CHK16-NEXT: Barycentrics

// CHECK: ; SV_Barycentrics
// CHECK: ; SV_Barycentrics
// CHECK: ; SV_Barycentrics
// CHECK: ; SV_Barycentrics

float4 main(float3 bary : SV_Barycentrics, noperspective float3 bary1 : SV_Barycentrics1) : SV_Target
{
  float4 vcolor0 = float4(1,0,0,1);
  float4 vcolor1 = float4(0,1,0,1);
  float4 vcolor2 = float4(0,0,0,1);
  float4 vcolorPerspective =  bary.x * vcolor0 + bary.y * vcolor1 + bary.z * vcolor2;
  float4 vcolorNoPerspectiveCentroid = bary1.x * vcolor0 + bary1.y * vcolor1 + bary1.z * vcolor2;
  return (vcolorPerspective + vcolorNoPerspectiveCentroid) / 2;
}