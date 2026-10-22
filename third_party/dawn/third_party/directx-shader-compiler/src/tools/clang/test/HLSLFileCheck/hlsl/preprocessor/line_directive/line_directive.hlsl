// RUN: %dxc -E main -T ps_6_0  %s | FileCheck %s

// #line directive should be honored
// CHECK: renamed.hlsl:42

float4 main() : SV_Target
{
#line 42 "renamed.hlsl"
  return foo; // Undeclared identifier
}