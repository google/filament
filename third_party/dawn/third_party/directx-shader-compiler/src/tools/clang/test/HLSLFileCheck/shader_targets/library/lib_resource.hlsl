// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure globals for resource exist.
// CHECK: @"\01?g_txDiffuse{{[@$?.A-Za-z0-9_]+}}" = external constant %"class.Texture2D<vector<float, 4> >", align 4
// CHECK: @"\01?g_samLinear{{[@$?.A-Za-z0-9_]+}}" = external constant %struct.SamplerState, align 4

Texture2D    g_txDiffuse;
SamplerState    g_samLinear;

float4 test(float2 c : C) : SV_TARGET
{
  float4 x = g_txDiffuse.Sample( g_samLinear, c );
  return x;
}

