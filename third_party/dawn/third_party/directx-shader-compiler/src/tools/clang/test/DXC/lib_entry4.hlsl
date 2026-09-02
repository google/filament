// Smoke test for dxl command line
// RUN: %dxc /T lib_6_x %s -Fo %t.lib_entry4.dxbc
// RUN: %dxc /T lib_6_x %S/Inputs/lib_res_match.hlsl -Fo %t.lib_res_match.dxbc
// RUN: %dxl -T ps_6_0 "%t.lib_entry4.dxbc;%t.lib_res_match.dxbc" -Fo %t.res_match_entry.dxbc
// RUN: %dxc -dumpbin %t.res_match_entry.dxbc | FileCheck %s

// CHECK:; cbuffer A
// CHECK-NEXT:; {
// CHECK-NEXT:;
// CHECK-NEXT:;   struct A
// CHECK-NEXT:;   {
// CHECK-NEXT:;
// CHECK-NEXT:float a;                                      ; Offset:    0
// CHECK-NEXT:float v;                                      ; Offset:    4
// CHECK-NEXT:;
// CHECK-NEXT:;   } A;

// Make sure same cbuffer decalred in different lib works.


cbuffer A {
  float a;
  float v;
}

Texture2D	tex;
SamplerState	samp;

float GetV();

[shader("pixel")]
float4 main() : SV_Target
{
   return tex.Sample(samp, float2(a, GetV()));
}
